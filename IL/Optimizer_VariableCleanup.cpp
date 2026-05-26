#include "Optimization.h"
#include <unordered_map>
#include <vector>

namespace Compiler
{
	namespace Intermediate
	{
		//第二柱！
		class VariableCleanupOptimizer : public IntraProcOptimizer
		{
		public:
			virtual ProgramOptimizationResult Optimize(std::shared_ptr<ControlFlowGraph> program) override
			{
				ProgramOptimizationResult rs;
				rs.Program = program;
				std::unordered_map<Variable*, InstructionNode *> referenced_variables;
				for (auto & node : program->Nodes)
				{
					for (auto instrNode = FirstInstructionNode(node->Code); instrNode; instrNode = NextInstructionNode(instrNode))
					{
						auto & instr = instrNode->Value;
						if (instr.LeftOperand.IsVariable())
						{
							referenced_variables[instr.LeftOperand.Var] = instrNode;
						}
						for (auto & op : instr.Operands)
						{
							if (op.IsVariable())
							{
								referenced_variables.emplace(op.Var, nullptr);
							}
						}
					}
				}
				program->VarDefs.clear();
				std::vector<std::shared_ptr<Variable>> newVars;
				for (auto & var : program->Variables)
				{
					if (var->Id < program->ParameterCount)
					{
						newVars.push_back(std::move(var));
						program->VarDefs.push_back(nullptr);
					}
					else if (auto iter = referenced_variables.find(var.get()); iter != referenced_variables.end())
					{
						var->Id = static_cast<int>(newVars.size());
						newVars.push_back(std::move(var));
						program->VarDefs.push_back(iter->second);
					}
					else
						rs.Changed = true;
				}
				program->Variables = std::move(newVars);
				return rs;
			}
		};
		// Remove variables that are never referenced after previous optimization passes.
		std::unique_ptr<IntraProcOptimizer> CreateVariableCleanupOptimizer()
		{
			return std::make_unique<VariableCleanupOptimizer>();
		}
	}
}
