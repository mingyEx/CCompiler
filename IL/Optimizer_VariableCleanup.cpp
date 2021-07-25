#include "Optimization.h"

namespace Compiler
{
	namespace Intermediate
	{
		//第二柱！
		class VariableCleanupOptimizer : public IntraProcOptimizer
		{
		public:
			virtual ProgramOptimizationResult Optimize(RefPtr<ControlFlowGraph> program) override
			{
				ProgramOptimizationResult rs;
				rs.Program = program;
				Dictionary<Variable*, InstructionNode *> referencedVariables;
				for (auto & node : program->Nodes)
				{
					for (auto instrNode = node->Code.FirstNode(); instrNode; instrNode = instrNode->GetNext())
					{
						auto & instr = instrNode->Value;	//是迭代器里的具体一条指令. Instruction
						if (instr.LeftOperand.IsVariable())
						{
							referencedVariables[instr.LeftOperand.Var] = instrNode;	// 右边是存了Instruction 的在list的一个Node. 左边是，结果.
						}
						for (auto & op : instr.Operands)
						{
							if (op.IsVariable())
							{
								referencedVariables.AddIfNotExists(op.Var, 0);	//如果操作数里有不存在于此dict的就加进去.
							}
						}
					}
				}
				program->VarDefs.Clear();	//先清空变量定义集合。 为什么是List<InstructionNode *>?
				List<RefPtr<Variable>> newVars;
				for (auto & var : program->Variables)	// 遍历 List<RefPtr<Variable>>
				{
					InstructionNode * def;
					if (var->Id < program->ParameterCount)	//如果指向变量的此指针的Id 小于 形参数量，则说明？
					{
						newVars.Add(_Move(var));		//总之就塞到新变量里.
						program->VarDefs.Add(0);		//并且这个不知道是.. 哦，对于变量的定义的指令集合  为什么要加0 ? 大概还是占位.
					}
					else if (referencedVariables.TryGetValue(var.Ptr(), def))	//def依旧是返回值。
					{
						var->Id = newVars.Count();
						newVars.Add(_Move(var));
						program->VarDefs.Add(def);
					}
					else
						rs.Changed = true;		//干嘛用的，有地方会用到它？
				}
				program->Variables = _Move(newVars);
				return rs;
			}
		};
		//看起来就是把指令里的东西都过了一遍，然后在dict里求值，遇到可以被求值的就求值，并且加入新的容器里，否则就.. 把标志位设置为欸true? 有啥用？
		IntraProcOptimizer * CreateVariableCleanupOptimizer()
		{
			return new VariableCleanupOptimizer();
		}
	}
}