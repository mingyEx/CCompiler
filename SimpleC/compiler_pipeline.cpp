#include "compiler_pipeline.h"

#include "CodeGenerator.h"
#include "IL/CFG.h"
#include "IL/Optimization.h"
#include "IL/x86CodeGen.h"
#include "Lexer.h"
#include "Parser.h"
#include "SyntaxVisitors.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

using namespace SimpleC::Compiler;

namespace
{
	std::filesystem::path ReplaceExtension(
		const std::filesystem::path& path,
		const wchar_t* new_extension)
	{
		auto updated_path = path;
		updated_path.replace_extension(new_extension);
		return updated_path;
	}

	std::filesystem::path BuildFunctionDumpPath(
		const std::filesystem::path& base_path,
		const std::wstring& function_name,
		const wchar_t* suffix)
	{
		const auto file_name = base_path.wstring() + L"." + function_name + suffix;
		return std::filesystem::path(file_name);
	}

	std::wstring ReadSourceFile(const std::filesystem::path& input_path)
	{
		std::ifstream input(input_path, std::ios::binary);
		if (!input)
		{
			throw std::runtime_error("Failed to open source file.");
		}

		const std::string content{
			std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>()};

		std::wstring source;
		source.reserve(content.size());
		for (const unsigned char ch : content)
		{
			source.push_back(static_cast<wchar_t>(ch));
		}
		return source;
	}

	bool PrintErrors(std::vector<::Compiler::CompileError>& error_list)
	{
		if (error_list.empty())
		{
			return false;
		}

		for (int index = 0; index < static_cast<int>(error_list.size()); ++index)
		{
			auto& error = error_list[index];
			std::wcerr
				<< error.Message
				<< L" line "
				<< error.Line
				<< L", col "
				<< error.Col
				<< L'\n';
		}

		return true;
	}

	std::unique_ptr<Compiler::Intermediate::IntraProcOptimizer> CreateOptimizationPipeline()
	{
		using namespace Compiler::Intermediate;

		return std::make_unique<CompoundOptimizer>(
			std::make_unique<IterateOptimizer>(
				std::make_unique<CompoundOptimizer>(
					CreateUselessInstructionOptimizer(),
					CreateVariableCleanupOptimizer(),
					CreateDeadCodeOptimizer(),
					CreateControlFlowCleanupOptimizer(),
					CreateConstIndirectionRemovalOptimizer(),
					CreateVariableCleanupOptimizer())),
			CreatePeepHoleOptimizer());
	}

	std::unique_ptr<Compiler::Intermediate::IntraProcOptimizer> CreateRegisterAllocationPipeline()
	{
		using namespace Compiler::Intermediate;

		return std::make_unique<CompoundOptimizer>(
			CreateBranchFuseOptimizer(),
			CreateOutOfSSA_Transform(),
			CreateRegisterAllocator());
	}
}

namespace SimpleC
{
	namespace Compiler
	{
		bool CompileSourceFile(const std::filesystem::path& input_path)
		{
			const std::wstring file_name = input_path.wstring();
			const std::wstring source = ReadSourceFile(input_path);

			Lexer lexer;
			std::vector<::Compiler::CompileError> error_list;
			auto tokens = lexer.Parse(file_name, source, error_list);

			Parser parser(tokens, error_list, file_name);
			auto program_syntax_node = parser.Parse();
			if (PrintErrors(error_list))
			{
				return false;
			}

			auto semantics_visitor = CreateSemanticsVisitor(error_list);
			program_syntax_node->Accept(*semantics_visitor);
			if (PrintErrors(error_list))
			{
				return false;
			}

			CodeGenerator code_generator;
			program_syntax_node->Accept(code_generator);

			const auto code_path = ReplaceExtension(input_path, L".code");
			const auto cfg_path = ReplaceExtension(input_path, L".cfg");
			const auto asm_path = ReplaceExtension(input_path, L".asm");
			code_generator.CompiledCode->Dump(code_path);

			auto pre_ssa_optimizer = Compiler::Intermediate::CreateControlFlowCleanupOptimizer();
			auto optimizer = CreateOptimizationPipeline();
			auto register_allocator = CreateRegisterAllocationPipeline();

			for (auto& function : code_generator.CompiledCode->Functions)
			{
				std::shared_ptr<Compiler::Intermediate::ControlFlowGraph> graph =
					Compiler::Intermediate::ControlFlowGraph::FromCode(function);

				graph->Dump(BuildFunctionDumpPath(cfg_path, function.Name, L"_original.cfgdump"));
				graph = pre_ssa_optimizer->Optimize(graph).Program;

				graph->ConvertToSSA();
				graph->Dump(BuildFunctionDumpPath(cfg_path, function.Name, L"_ssa.cfgdump"));

				graph = optimizer->Optimize(graph).Program;
				graph->Dump(BuildFunctionDumpPath(cfg_path, function.Name, L"_optimize.cfgdump"));

				graph = register_allocator->Optimize(graph).Program;
				graph->ToCode(function);
				graph->Dump(BuildFunctionDumpPath(cfg_path, function.Name, L"_final.cfgdump"));
			}

			auto x86_generator = Compiler::Intermediate::CreateX86CodeGenerator();
			auto program = x86_generator->GenerateCode(code_generator.CompiledCode.get());
			for (auto& function : program.Functions)
			{
				function.Dump(BuildFunctionDumpPath(asm_path, function.Name, L".txt"));
			}

			program.Link();
			return true;
		}

		int RunCompilerFromCommandLine(int argc, wchar_t* argv[])
		{
			if (argc != 2)
			{
				std::wcerr << L"Usage: SimpleC <source-file>" << std::endl;
				return 1;
			}

			try
			{
				return CompileSourceFile(argv[1]) ? 0 : 1;
			}
			catch (const std::exception& error)
			{
				std::cerr << "Compiler failed: " << error.what() << '\n';
				return 1;
			}
			catch (const CoreLib::Basic::Exception& error)
			{
				std::cerr << "Compiler failed: " << error.Message.ToMultiByteString() << '\n';
				return 1;
			}
		}
	}
}
