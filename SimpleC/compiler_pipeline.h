#ifndef SIMPLE_C_COMPILER_PIPELINE_H
#define SIMPLE_C_COMPILER_PIPELINE_H

#include <filesystem>

namespace SimpleC
{
	namespace Compiler
	{
		int RunCompilerFromCommandLine(int argc, wchar_t* argv[]);
		bool CompileSourceFile(const std::filesystem::path& input_path);
	}
}

#endif
