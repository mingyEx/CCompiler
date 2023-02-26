#include "Basic.h"
#include "Lexer.h"
#include "Parser.h"
#include "LibIO.h"
#include "SyntaxVisitors.h"
#include "CodeGenerator.h"
#include "IL/CFG.h"
#include "IL/Optimization.h"
#include "IL/x86CodeGen.h"
#include<iostream>
#include<filesystem>
using namespace CoreLib::Basic;
using namespace SimpleC::Compiler;

void PrintError(List<CompileError> &errorList)
{
	if (errorList.Count() > 0)
	{
		for (int i = 0; i<errorList.Count(); i++)
		{
			printf("%s line %d, col %d\n", errorList[i].Message.ToMultiByteString(), errorList[i].Line, errorList[i].Col);
		}
	}
}

//函数类型的别名
typedef int (__stdcall *IntFunc)();	

int __stdcall sum()
{
	int sum = 0;
	for (int i = 1; i<9; i++)
		for (int j = 0; j<9; j++)
			if ((i*j%2) == 0)
				sum= sum + i*j; 
	return sum;
}

int __stdcall hannoi(int level)
{
	int rs = 0;
	if (level==1)
		return 1;
	return hannoi(level-1)*2 + 1;
}

//返回一个函数指针，功能是把内容写到二进制文件里。
IntFunc MakeFunc(List<unsigned char> & code)	
{
	char * rs = (char *)VirtualAlloc(0, code.Count(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	memcpy(rs, code.Buffer(), code.Count());
	FlushInstructionCache(GetCurrentProcess(), rs, code.Count());
	return (IntFunc)(rs);
}

int wmain(int argc, wchar_t* argv[])
{
	if (argc == 2)	//接收文件
	{
		String fileName = argv[1];
		auto source = File::ReadAllText(fileName);
		Lexer lexer;
		List<CompileError> errorList;

		auto tokens = lexer.Parse(fileName, source, errorList);	//状态机，先不管.
		//for (auto i : tokens) std::wcout << i.Content.Buffer() << std::endl;

		Parser parser(tokens, errorList, fileName);
		auto programSyntaxNode = parser.Parse();	//return the total file' SyntaxNode,which include all the functionNode.
		if (errorList.Count() > 0)
		{
			PrintError(errorList);
			return 0;
		}

		//Simple type checking
		RefPtr<SyntaxVisitor> visitor = CreateSemanticsVisitor(errorList);		
		programSyntaxNode->Accept(visitor.Ptr());		
		
		if (errorList.Count() > 0)
		{
			PrintError(errorList);
			return 0;
		}

		//print stmt
		//RefPtr<SyntaxVisitor> visitors = CreateSyntaxPrinter();
		//programSyntaxNode->Accept(visitors.Ptr());

		SimpleC::Compiler::CodeGenerator gen;
		programSyntaxNode->Accept(&gen);
		fileName = Path::ReplaceExt(fileName, L"code");	
		gen.CompiledCode->Dump(fileName);

		RefPtr<IntraProcOptimizer> preSsaOptmizer = CreateControlFlowCleanupOptimizer();
		RefPtr<IntraProcOptimizer> optimizer =	//Intra Proc Optimizer:base of others.
			new CompoundOptimizer	
			(new IterateOptimizer
				(
					new CompoundOptimizer
					(
						CreateUselessInstructionOptimizer(),
						CreateVariableCleanupOptimizer(),
						CreateDeadCodeOptimizer(),
						CreateControlFlowCleanupOptimizer(),//控制流清理,与上述几乎重合，除了一个while()来反复多次直到无变化.
						CreateConstIndirectionRemovalOptimizer(),//常量间接删除优化器,书上没有
						CreateVariableCleanupOptimizer()
					)),	
				CreatePeepHoleOptimizer()		//窥孔优化，只实现了最基本的指令重写，没有先进的微语言和算法。
			);
		RefPtr<IntraProcOptimizer> regAllocator =
			new CompoundOptimizer(
				CreateBranchFuseOptimizer(),	//分支融合优化,没懂，代码很少.
				CreateOutOfSSA_Transform(),	//转出ssa. 
				CreateRegisterAllocator()	//寄存器分配
			);
		
		for (auto& func : gen.CompiledCode->Functions)	//对每个函数进行的。  跨基本块的优化呢？
		{
			RefPtr<ControlFlowGraph> graph = ControlFlowGraph::FromCode(func);		//三地址代码->cfg.
			graph->Dump(Path::ReplaceExt(fileName, L"cfg") + L"." + func.Name + L"_original.cfgdump");	//cfg中信息以二进制格式写入.
			
			//删除phi指令的? 
			graph = preSsaOptmizer->Optimize(graph).Program;

			graph->ConvertToSSA();
			graph->Dump(Path::ReplaceExt(fileName, L"cfg") + L"." + func.Name + L"_ssa.cfgdump");

			graph = optimizer->Optimize(graph).Program;
			graph->Dump(Path::ReplaceExt(fileName, L"cfg") + L"." + func.Name + L"_optimize.cfgdump");

			//分支合并，转出SSA，寄存器分配的优化器
			graph = regAllocator->Optimize(graph).Program;

			//把cfg里的指令都重新塞到函数里.
			graph->ToCode(func);
			graph->Dump(Path::ReplaceExt(fileName, L"cfg") + L"." + func.Name + L"_final.cfgdump");
		}
		
		//指令生成,书上11章.
		RefPtr<X86CodeGenerator> x86Gen = CreateX86CodeGenerator();	//对着指令生成对应的汇编语言。 
		auto program = x86Gen->GenerateCode(gen.CompiledCode.Ptr());
		for (auto & func : program.Functions)
		{
			func.Dump(Path::ReplaceExt(fileName, L"asm") + L"." + func.Name + L".txt");	//POP ECX etc.
		}
		
		auto assembly = program.Link();	
		auto exe = assembly.CreateMemoryExecutable();	//到此为止生成了啥?
		IntFunc f = (IntFunc)exe.Buffer;
	}
	else
		std::wcout << "no input" << std::endl;
	return 0;
}