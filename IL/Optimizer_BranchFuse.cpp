#include "Optimization.h"

namespace Compiler
{
	namespace Intermediate
	{
		//分支融合优化
		class BranchFuseOptimizer : public IntraProcOptimizer
		{
		public:
			virtual ProgramOptimizationResult Optimize(RefPtr<ControlFlowGraph> program) override
			{
				ProgramOptimizationResult rs;
				rs.Program = program;
				rs.Changed = false;
				List<int> varRefs;
				varRefs.SetSize(program->Variables.Count());
				memset(varRefs.Buffer(), 0, sizeof(int)*varRefs.Count());
				for (auto node : program->Nodes)
					for (auto & instr : node->Code)
						for (auto & op : instr.Operands)
							if (op.IsVariable())
								varRefs[op.Var->Id]++;
				for (auto node : program->Nodes)
					for (auto instrNode = node->Code.FirstNode(); instrNode; instrNode=instrNode->GetNext())
					{
						auto &instr = instrNode->Value;
						if (instr.Func == Operation::Branch && instr.Operands[2].IntValue < 2)
						{
							if (instr.Operands[0].IsVariable() && varRefs[instr.Operands[0].Var->Id] == 1)
							{
								auto prevInstr = instrNode->GetPrevious();
								if (!prevInstr || prevInstr->Value.LeftOperand.Var != instr.Operands[0].Var)
									continue;
								rs.Changed = true;
								
								// 0 : jt, 1: jf, 2: jl, 3:lge, 4: jg, 5: jle, 6: je, 7:jne
								int & branchType = instr.Operands[2].IntValue;
								if (prevInstr->Value.Func == Operation::Less)
								{
									prevInstr->Value.Func = Operation::Compare;
									branchType = branchType?3:2;
								}
								else if (prevInstr->Value.Func == Operation::Leq)
								{
									prevInstr->Value.Func = Operation::Compare;
									branchType = branchType?4:5;
								}
								else if (prevInstr->Value.Func == Operation::Geq)
								{
									prevInstr->Value.Func = Operation::Compare;
									branchType = branchType?2:3;
								}
								else if (prevInstr->Value.Func == Operation::Greater)
								{
									prevInstr->Value.Func = Operation::Compare;
									branchType = branchType?5:4;
								}
								else if (prevInstr->Value.Func == Operation::Eql)
								{
									prevInstr->Value.Func = Operation::Compare;
									branchType = branchType?7:6;
								}
								else if (prevInstr->Value.Func == Operation::Neq)
								{
									prevInstr->Value.Func = Operation::Compare;
									branchType = branchType?6:7;
								}
								else
									continue;
								instr.Operands[0].Var->Size = 0;
							}
						}
					}
				return rs;
			}
		};

		IntraProcOptimizer * CreateBranchFuseOptimizer()
		{
			return new BranchFuseOptimizer();
		}
	}
}