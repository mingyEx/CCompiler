#ifndef COMPILER_OPTIMIZATIONS_H
#define COMPILER_OPTIMIZATIONS_H

#include "Basic.h"
#include "CFG.h"

namespace Compiler
{
	namespace Intermediate
	{
		using namespace CoreLib::Basic;

		//优化结果，ptr和flag.
		struct ProgramOptimizationResult
		{
			RefPtr<ControlFlowGraph> Program;
			bool Changed;
			ProgramOptimizationResult()
			{
				Changed = false;
			}
		};
		
		//其他优化器的基类
		class IntraProcOptimizer : public Object
		{
		public:
			virtual ProgramOptimizationResult Optimize(RefPtr<ControlFlowGraph> program)=0;
		};
		
		//优化器打包，与其他的无关.
		class CompoundOptimizer : public IntraProcOptimizer
		{
		public:
			List<RefPtr<IntraProcOptimizer>> Optimizers;

			//参数类型都一样，可以改成变参模板类型吗?需要支持递归调用。 关于这个可以看:https://stackoverflow.com/a/60209012/13792395
			//首先要有一个模板... 这个你就已经不及格了，放弃吧.
			CompoundOptimizer()
			{}
			CompoundOptimizer(RefPtr<IntraProcOptimizer> op1)
			{
				Optimizers.Add(op1);
			}
			CompoundOptimizer(RefPtr<IntraProcOptimizer> op1, RefPtr<IntraProcOptimizer> op2)
			{
				Optimizers.Add(op1);
				Optimizers.Add(op2);
			}
			CompoundOptimizer(RefPtr<IntraProcOptimizer> op1, RefPtr<IntraProcOptimizer> op2, RefPtr<IntraProcOptimizer> op3)
			{
				Optimizers.Add(op1);
				Optimizers.Add(op2);
				Optimizers.Add(op3);
			}
			CompoundOptimizer(RefPtr<IntraProcOptimizer> op1, RefPtr<IntraProcOptimizer> op2, RefPtr<IntraProcOptimizer> op3, RefPtr<IntraProcOptimizer> op4)
			{
				Optimizers.Add(op1);
				Optimizers.Add(op2);
				Optimizers.Add(op3);
				Optimizers.Add(op4);
			}
			CompoundOptimizer(RefPtr<IntraProcOptimizer> op1, RefPtr<IntraProcOptimizer> op2, RefPtr<IntraProcOptimizer> op3, RefPtr<IntraProcOptimizer> op4, RefPtr<IntraProcOptimizer> op5)
			{
				Optimizers.Add(op1);
				Optimizers.Add(op2);
				Optimizers.Add(op3);
				Optimizers.Add(op4);
				Optimizers.Add(op5);
			}
			CompoundOptimizer(RefPtr<IntraProcOptimizer> op1, RefPtr<IntraProcOptimizer> op2, RefPtr<IntraProcOptimizer> op3, RefPtr<IntraProcOptimizer> op4, RefPtr<IntraProcOptimizer> op5, RefPtr<IntraProcOptimizer> op6)
			{
				Optimizers.Add(op1);
				Optimizers.Add(op2);
				Optimizers.Add(op3);
				Optimizers.Add(op4);
				Optimizers.Add(op5);
				Optimizers.Add(op6);
			}
			CompoundOptimizer(RefPtr<IntraProcOptimizer> op1, RefPtr<IntraProcOptimizer> op2, RefPtr<IntraProcOptimizer> op3, RefPtr<IntraProcOptimizer> op4, RefPtr<IntraProcOptimizer> op5, RefPtr<IntraProcOptimizer> op6, RefPtr<IntraProcOptimizer> op7)
			{
				Optimizers.Add(op1);
				Optimizers.Add(op2);
				Optimizers.Add(op3);
				Optimizers.Add(op4);
				Optimizers.Add(op5);
				Optimizers.Add(op6);
				Optimizers.Add(op7);
			}
			CompoundOptimizer(RefPtr<IntraProcOptimizer> op1, RefPtr<IntraProcOptimizer> op2, RefPtr<IntraProcOptimizer> op3, RefPtr<IntraProcOptimizer> op4, RefPtr<IntraProcOptimizer> op5, RefPtr<IntraProcOptimizer> op6, RefPtr<IntraProcOptimizer> op7, RefPtr<IntraProcOptimizer> op8)
			{
				Optimizers.Add(op1);
				Optimizers.Add(op2);
				Optimizers.Add(op3);
				Optimizers.Add(op4);
				Optimizers.Add(op5);
				Optimizers.Add(op6);
				Optimizers.Add(op7);
				Optimizers.Add(op8);
			}

			virtual ProgramOptimizationResult Optimize(RefPtr<ControlFlowGraph> program) override	// 语法复习: override确保该函数为虚函数并覆盖某个基类中的虚函数
			{
				ProgramOptimizationResult rs;
				rs.Program = program;
				for (auto & optimizer : Optimizers)
				{
					auto partialRs = optimizer->Optimize(rs.Program);
					rs.Program = partialRs.Program;
					rs.Changed |= partialRs.Changed;
				}
				return rs;
			}
		};

		class IterateOptimizer : public IntraProcOptimizer
		{
		public:
			RefPtr<IntraProcOptimizer> Optimizer;
			IterateOptimizer(){}
			IterateOptimizer(RefPtr<IntraProcOptimizer> optimizer)
			{
				this->Optimizer = optimizer;
			}
			virtual ProgramOptimizationResult Optimize(RefPtr<ControlFlowGraph> program) override
			{
				bool changed = false;
				ProgramOptimizationResult rs;
				rs.Program = program;
				do
				{
					rs = Optimizer->Optimize(rs.Program);	//调用的是 BranchFuseOptimizer的Optimize.
					if (rs.Changed)	//flag.
						changed = true;
				}
				while (rs.Changed);
				rs.Changed = changed;
				return rs;
			}
		};
		//剩下的就是各个对应的.cpp文件的具体实现了.
		IntraProcOptimizer * CreateDeadCodeOptimizer();
		IntraProcOptimizer * CreateUselessInstructionOptimizer();
		IntraProcOptimizer * CreateVariableCleanupOptimizer();
		IntraProcOptimizer * CreateConstIndirectionRemovalOptimizer();
		IntraProcOptimizer * CreateOutOfSSA_Transform();
		IntraProcOptimizer * CreateRegisterAllocator();
		IntraProcOptimizer * CreateBranchFuseOptimizer();
		IntraProcOptimizer * CreateControlFlowCleanupOptimizer();
		IntraProcOptimizer* CreatePeepHoleOptimizer();
	}
}

#endif