#ifndef COMPILER_OPTIMIZATIONS_H
#define COMPILER_OPTIMIZATIONS_H

#include <memory>
#include <utility>
#include <vector>

#include "CFG.h"

namespace Compiler
{
	namespace Intermediate
	{
		//优化结果，ptr和flag.
		struct ProgramOptimizationResult
		{
			std::shared_ptr<ControlFlowGraph> Program;
			bool Changed;
			ProgramOptimizationResult()
			{
				Changed = false;
			}
		};
		
		//其他优化器的基类
		class IntraProcOptimizer
		{
		public:
			virtual ~IntraProcOptimizer() = default;
			virtual ProgramOptimizationResult Optimize(std::shared_ptr<ControlFlowGraph> program)=0;
		};
		
		//优化器打包，与其他的无关.
		class CompoundOptimizer : public IntraProcOptimizer
		{
		public:
			std::vector<std::unique_ptr<IntraProcOptimizer>> Optimizers;

			//参数类型都一样，可以改成变参模板类型吗?需要支持递归调用。 关于这个可以看:https://stackoverflow.com/a/60209012/13792395
			//首先要有一个模板... 这个就已经不及格了，放弃.
			CompoundOptimizer()
			{}
			void AddOptimizer(std::unique_ptr<IntraProcOptimizer> optimizer)
			{
				Optimizers.push_back(std::move(optimizer));
			}
			template <typename... TOptimizers>
			explicit CompoundOptimizer(TOptimizers&&... optimizers)
			{
				(AddOptimizer(std::forward<TOptimizers>(optimizers)), ...);
			}

			virtual ProgramOptimizationResult Optimize(std::shared_ptr<ControlFlowGraph> program) override	// 语法复习: override确保该函数为虚函数并覆盖某个基类中的虚函数
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
			std::unique_ptr<IntraProcOptimizer> Optimizer;
			IterateOptimizer(){}
			explicit IterateOptimizer(std::unique_ptr<IntraProcOptimizer> optimizer)
				: Optimizer(std::move(optimizer))
			{
			}
			virtual ProgramOptimizationResult Optimize(std::shared_ptr<ControlFlowGraph> program) override
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
		std::unique_ptr<IntraProcOptimizer> CreateDeadCodeOptimizer();
		std::unique_ptr<IntraProcOptimizer> CreateUselessInstructionOptimizer();
		std::unique_ptr<IntraProcOptimizer> CreateVariableCleanupOptimizer();
		std::unique_ptr<IntraProcOptimizer> CreateConstIndirectionRemovalOptimizer();
		std::unique_ptr<IntraProcOptimizer> CreateOutOfSSA_Transform();
		std::unique_ptr<IntraProcOptimizer> CreateRegisterAllocator();
		std::unique_ptr<IntraProcOptimizer> CreateBranchFuseOptimizer();
		std::unique_ptr<IntraProcOptimizer> CreateControlFlowCleanupOptimizer();
		std::unique_ptr<IntraProcOptimizer> CreatePeepHoleOptimizer();
	}
}

#endif
