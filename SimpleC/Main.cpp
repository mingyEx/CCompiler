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
//这是一个无聊的测试，Do you hear me?
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

		auto tokens = lexer.Parse(argv[1], source, errorList);	//fileName作为错误输出

		Parser parser(tokens, errorList, fileName);
		auto programSyntaxNode = parser.Parse();	//返回的是一些函数组成的list的头节点的指针。
		if (errorList.Count() > 0)
		{
			PrintError(errorList);
			return 0;
		}
		//访问者模式的经典使用场景，语义分析器只进行了类型检查.
		RefPtr<SyntaxVisitor> visitor = CreateSemanticsVisitor(errorList);		
		programSyntaxNode->Accept(visitor.Ptr());		
		
		//programSyntaxNode 是一个指针，指向一系列函数组成的列表，列表是程序体的结点指针。

		if (errorList.Count() > 0)
		{
			PrintError(errorList);
			return 0;
		}

		//打印文本:
		//RefPtr<SyntaxVisitor> visitors = CreateSyntaxPrinter();
		//programSyntaxNode->Accept(visitors.Ptr());

		SimpleC::Compiler::CodeGenerator gen;	//代码生成是从语法树传给代码生成器的。
		programSyntaxNode->Accept(&gen);
		fileName = Path::ReplaceExt(fileName, L"code");	//生成三地址代码
		gen.CompiledCode->Dump(fileName);

		//优化器声明。
		RefPtr<IntraProcOptimizer> preSsaOptmizer = CreateControlFlowCleanupOptimizer();
		RefPtr<IntraProcOptimizer> optimizer =	//Intra Proc Optimizer，内部处理优化器，作为其他优化器的基类。
			new CompoundOptimizer	
			(
				new IterateOptimizer
				(
					new CompoundOptimizer
					(
						CreateUselessInstructionOptimizer(),//创建无用的指令优化器
						CreateVariableCleanupOptimizer(),	//创建变量清理优化器
						CreateDeadCodeOptimizer(),			//死代码消除
						CreateControlFlowCleanupOptimizer(),//控制流清理
						//与上述几乎重合，除了一个while()来反复多次直到无变化.
						CreateConstIndirectionRemovalOptimizer(),//创建常量间接删除优化器,书上没有
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

			//功能更强大的优化器,六御锁魂阵!
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

		//下面是用来测量此程序执行时间的？
		//可是为什么到这里才开始出现计时，却统计的是全部时间？
		//我发现无论编译什么都是那么长的时间，作者可能是个鸡贼。
		//	LARGE_INTEGER c1, c2, freq;
		//	int rs = 0;
		//	int rs2=0;
		//	QueryPerformanceCounter(&c1);
		//	for (int i = 0; i<1000; i++)
		//	//	rs = f();
		//	QueryPerformanceCounter(&c2);
		//	QueryPerformanceFrequency(&freq);
		//	double time1 = (c2.QuadPart-c1.QuadPart)/(double)(freq.QuadPart);
		//	printf("time1: %f\n", time1);
		//	QueryPerformanceCounter(&c1);
		//	for (int i = 0; i<1000; i++)
		//		rs2 = sum();
		//	QueryPerformanceCounter(&c2);
		//	double time2 = (c2.QuadPart-c1.QuadPart)/(double)(freq.QuadPart);
		//	printf("time2: %f\n", time2);
		//	printf("result is %d, %d\n", rs, rs2);
		//	PrintError(errorList);
	}
	return 0;
}