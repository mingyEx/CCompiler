#include "Optimization.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Compiler
{
	namespace Intermediate
	{
		//大概是记录代码里对指针的使用的，offset和size都记录下来了.
		struct PointerUsage
		{
			int Offset;
			int Size;
			PointerUsage()
			{}
			PointerUsage(int offset, int size)
			{
				Offset = offset;
				Size = size;
			}
			int GetHashCode() const
			{
				return (Size<<8) + Offset;
			}
			bool operator == (const PointerUsage & p) const
			{
				return Offset == p.Offset && (Size == p.Size || Offset == -1);
			}
			bool operator != (const PointerUsage & p) const
			{
				return !(*this == p);
			}
		};

		struct PointerUsageHasher
		{
			size_t operator()(const PointerUsage& usage) const noexcept
			{
				return static_cast<size_t>(usage.GetHashCode());
			}
		};
		//那么这个玩意是为了记录，指针如果指向同一个值就折叠掉？ 
		class ConstIndirectionRemovalOptimizer : public IntraProcOptimizer
		{
		private:
			//记录指针表达式的值?
			struct PointerValue
			{
				Variable * BaseVar = nullptr;
				int Offset = -1;

				PointerValue() = default;
				PointerValue(const PointerValue & pv) = default;
				PointerValue(PointerValue && pv) = default;
				PointerValue& operator = (const PointerValue & pv) = default;
				PointerValue& operator = (PointerValue && pv) = default;
			};
			
			//对指针求值？
			void EvalPointers(
				std::unordered_map<Variable*, PointerValue>& dict,
				std::vector<std::unordered_set<PointerUsage, PointerUsageHasher>>& used_pointers,
				ControlFlowGraph* program)
			{
				std::unordered_set<Variable *> unknownPtrVars;
				auto traverse = program->GetPostOrder();
				for (int i = static_cast<int>(traverse.size()) - 1; i>=0; i--)
				{
					auto & node = traverse[i];
					for (auto & instr : node->Code)
					{
						if (unknownPtrVars.contains(instr.LeftOperand.Var))
						{
							used_pointers[instr.Operands[0].Var->Id].insert(PointerUsage(-1, 0));
						}
						if (instr.Func == Operation::Lea && instr.Operands[0].IsVariable())
						{
							PointerValue pv;
							pv.BaseVar = instr.Operands[0].Var;
							pv.Offset = 0;
							dict[instr.LeftOperand.Var] = std::move(pv);
						}
						else if (instr.Func == Operation::Add ||
								 instr.Func == Operation::Sub ||
								 instr.Func == Operation::SInc)
						{
							PointerValue pv;
							if (auto iter = dict.find(instr.Operands[0].Var); iter != dict.end())
							{
								pv = iter->second;
								if (pv.Offset != -1)
								{
									if (instr.Func == Operation::Add &&
										instr.Operands[1].IsIntegral())
										pv.Offset += instr.Operands[1].IntValue;
									else if (instr.Func == Operation::Sub &&
										instr.Operands[1].IsIntegral())
										pv.Offset -= instr.Operands[1].IntValue;
									else if (instr.Func == Operation::SInc &&
										instr.Operands[1].IsIntegral() &&
										instr.Operands[2].IsIntegral())
										pv.Offset += instr.Operands[1].IntValue * instr.Operands[2].IntValue;
									else
										pv.Offset = -1;
								}
								if (unknownPtrVars.contains(instr.LeftOperand.Var))
									pv.Offset = -1;
								dict[instr.LeftOperand.Var] = std::move(pv);
							}
						}
						else if (instr.Func == Operation::Phi)
						{
							bool hasPtr = false;
							bool hasUnknown = false;
							PointerValue rpv;
							for (auto & op : instr.Operands)
							{
								PointerValue pv;
								if (auto iter = dict.find(op.Var); iter != dict.end())
								{
									pv = iter->second;
									hasPtr = true;
									if (pv.BaseVar)
										used_pointers[pv.BaseVar->Id].insert(PointerUsage(-1, 0));
								}
								else
									hasUnknown = true;
							}
							if (hasPtr && hasUnknown)
							{
								for (auto & op : instr.Operands)
								{
									if (!dict.contains(op.Var))
										unknownPtrVars.insert(op.Var);
								}
								
							}
							if (hasPtr)
							{
								rpv.Offset = -1;
								rpv.BaseVar = 0;
								dict[instr.LeftOperand.Var] = std::move(rpv);
							}
						}
						else if (instr.Func == 0)
						{
							PointerValue pv;
							if (auto iter = dict.find(instr.Operands[0].Var); iter != dict.end())
							{
								pv = iter->second;
								dict[instr.LeftOperand.Var] = std::move(pv);
							}
						}
						if (instr.Func == Operation::Load ||
							instr.Func == Operation::Store ||
							instr.Func == Operation::Ret ||
							instr.Func == Operation::Call)
						{
							for (auto & op : instr.Operands)
							{
								PointerValue pv;
								if (op.IsVariable())
								{
									if (auto iter = dict.find(op.Var); iter != dict.end())
									{
										pv = iter->second;
										int offset = pv.Offset;
										int size = 0;
										if (instr.Func == Operation::Load)
											size = GetTypeSize(instr.Operands[1].IntValue);
										else if (instr.Func == Operation::Store)
											size = instr.Operands[2].IntValue;
										else
										{
											size = 0;
											offset = -1;
										}
										if (pv.BaseVar)
											used_pointers[pv.BaseVar->Id].insert(PointerUsage(offset, size));
									}
								}
							}
						}
					}
				}
			}
		public:

			virtual ProgramOptimizationResult Optimize(std::shared_ptr<ControlFlowGraph> program) override
			{
				ProgramOptimizationResult rs;
				rs.Changed = false;
				rs.Program = program;
				std::unordered_map<Variable*, PointerValue> pointer_values;
				std::vector<std::unordered_map<int, Variable *>> replace_vars;
				std::vector<std::unordered_set<PointerUsage, PointerUsageHasher>> used_pointers;
				std::vector<PointerUsage> blocks;
				used_pointers.resize(program->Variables.size());
				replace_vars.resize(program->Variables.size());
				EvalPointers(pointer_values, used_pointers, program.get());
				for (int i = 0; i < static_cast<int>(used_pointers.size()); i++)
				{
					if (used_pointers[i].empty())
						continue;
					if (!used_pointers[i].contains(PointerUsage(-1, 0)))
					{
						blocks.clear();
						bool independentAccess = true;
						for (auto& b : used_pointers[i])
						{
							bool found = false;
							for (auto & eb : blocks)
							{
								if (b==eb)
									found = true;
								else if (b.Offset >= eb.Offset && b.Offset+b.Size <=eb.Offset+eb.Size)
								{
									independentAccess = false;
									break;
								}
							}
							if (!independentAccess)
								break;
							blocks.push_back(b);
						}
						if (independentAccess)
						{
							// create a variable for each offset
							for (auto & b: blocks)
							{
								auto var = std::make_shared<Variable>();
								var->Id = static_cast<int>(program->Variables.size());
								var->Name = L"^buf_" + program->Variables[i]->Name + L"_" + std::to_wstring(b.Offset);
								var->Size = b.Size;
								program->Variables.push_back(var);
								program->VarDefs.push_back(nullptr);
								replace_vars[i][b.Offset] = var.get();
							}
							rs.Changed = true;
						}
					}
				}
				if (rs.Changed)
				{
					// replace load and stores with variables
					for (auto & node : program->Nodes)
						for (auto & instr : node->Code)
						{
							PointerValue pv;
							if (!instr.Operands.empty() && instr.Operands[0].IsVariable())
							{
								if (auto iter = pointer_values.find(instr.Operands[0].Var); iter != pointer_values.end())
								{
									pv = iter->second;
									if (instr.Func == Operation::Load)
									{
										instr.Func = 0;
										instr.Operands.clear();
										instr.Operands.push_back(replace_vars[pv.BaseVar->Id].at(pv.Offset));
									}
									else if (instr.Func == Operation::Store)
									{
										instr.Func = 0;
										auto opValue = instr.Operands[1];
										instr.Operands.clear();
										instr.Operands.push_back(opValue);
										instr.LeftOperand = Operand(replace_vars[pv.BaseVar->Id].at(pv.Offset));
									}
								}
							}
						}
				}
				return rs;
			}
		};
		std::unique_ptr<IntraProcOptimizer> CreateConstIndirectionRemovalOptimizer()
		{
			return std::make_unique<ConstIndirectionRemovalOptimizer>();
		}
	}
}
