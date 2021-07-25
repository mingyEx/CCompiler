#include "Optimization.h"

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
			int GetHashCode()
			{
				return (Size<<8) + Offset;
			}
			bool operator == (const PointerUsage & p)
			{
				return Offset == p.Offset && (Size == p.Size || Offset == -1);
			}
			bool operator != (const PointerUsage & p)
			{
				return !(*this == p);
			}
		};
		//那么这个玩意是为了记录，指针如果指向同一个值就折叠掉？ 
		class ConstIndirectionRemovalOptimizer : public IntraProcOptimizer
		{
		private:
			//记录指针表达式的值?
			struct PointerValue
			{
				Variable * BaseVar;
				int Offset;

				PointerValue()
				{
					Offset = -1;
				}
				PointerValue(const PointerValue & pv)
				{
					BaseVar = pv.BaseVar;
					Offset = pv.Offset;
				}
				PointerValue(PointerValue && pv)
				{
					BaseVar = _Move(pv.BaseVar);
					Offset = pv.Offset;
				}
				PointerValue& operator = (const PointerValue & pv)
				{
					BaseVar = pv.BaseVar;
					Offset = pv.Offset;
					return *this;
				}
				PointerValue& operator = (PointerValue && pv)
				{
					BaseVar = _Move(pv.BaseVar);
					Offset = pv.Offset;
					return *this;
				}
			};
			
			//对指针求值？
			void EvalPointers(Dictionary<Variable*, PointerValue> & dict, List<HashSet<PointerUsage>> & usedPointers, ControlFlowGraph * program)
			{
				HashSet<Variable *> unknownPtrVars;
				auto traverse = program->GetPostOrder();
				for (int i = traverse.Count()-1; i>=0; i--)
				{
					auto & node = traverse[i];
					for (auto & instr : node->Code)
					{
						if (unknownPtrVars.Contains(instr.LeftOperand.Var))
						{
							usedPointers[instr.Operands[0].Var->Id].Add(PointerUsage(-1,0));
						}
						if (instr.Func == Operation::Lea && instr.Operands[0].IsVariable())
						{
							PointerValue pv;
							pv.BaseVar = instr.Operands[0].Var;
							pv.Offset = 0;
							dict[instr.LeftOperand.Var] = _Move(pv);
						}
						else if (instr.Func == Operation::Add ||
								 instr.Func == Operation::Sub ||
								 instr.Func == Operation::SInc)
						{
							PointerValue pv;
							if (dict.TryGetValue(instr.Operands[0].Var, pv))
							{
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
								if (unknownPtrVars.Contains(instr.LeftOperand.Var))
									pv.Offset = -1;
								dict[instr.LeftOperand.Var] = _Move(pv);
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
								if (dict.TryGetValue(op.Var, pv))
								{
									hasPtr = true;
									if (pv.BaseVar)
										usedPointers[pv.BaseVar->Id].Add(PointerUsage(-1,0));
								}
								else
									hasUnknown = true;
							}
							if (hasPtr && hasUnknown)
							{
								for (auto & op : instr.Operands)
								{
									if (!dict.ContainsKey(op.Var))
										unknownPtrVars.Add(op.Var);
								}
								
							}
							if (hasPtr)
							{
								rpv.Offset = -1;
								rpv.BaseVar = 0;
								dict[instr.LeftOperand.Var] = _Move(rpv);
							}
						}
						else if (instr.Func == 0)
						{
							PointerValue pv;
							if (dict.TryGetValue(instr.Operands[0].Var, pv))
							{
								dict[instr.LeftOperand.Var] = _Move(pv);
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
								if (op.IsVariable() && dict.TryGetValue(op.Var, pv))
								{
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
										usedPointers[pv.BaseVar->Id].Add(PointerUsage(offset, size));
								}
							}
						}
					}
				}
			}
		public:

			virtual ProgramOptimizationResult Optimize(RefPtr<ControlFlowGraph> program) override
			{
				ProgramOptimizationResult rs;
				rs.Changed = false;
				rs.Program = program;
				Dictionary<Variable*, PointerValue> pointerValues;
				List<Dictionary<int, Variable *>> replaceVars;
				List<HashSet<PointerUsage>> usedPointers;
				List<PointerUsage> blocks;
				usedPointers.SetSize(program->Variables.Count());
				replaceVars.SetSize(program->Variables.Count());
				EvalPointers(pointerValues, usedPointers, program.Ptr());
				for (int i = 0; i<usedPointers.Count(); i++)
				{
					if (usedPointers[i].Count() == 0)
						continue;
					if (!usedPointers[i].Contains(PointerUsage(-1,0)))
					{
						blocks.Clear();
						bool independentAccess = true;
						for (auto & b : usedPointers[i])
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
							blocks.Add(b);
						}
						if (independentAccess)
						{
							// create a variable for each offset
							for (auto & b: blocks)
							{
								auto *var = new Variable();
								var->Id = program->Variables.Count();
								StringBuilder sb(48);
								sb.Append(L"^buf_");
								sb.Append(program->Variables[i]->Name);
								sb.Append(L"_");
								sb.Append(String(b.Offset));
								var->Name = sb.ProduceString();
								var->Size = b.Size;
								program->Variables.Add(var);
								program->VarDefs.Add(0);
								replaceVars[i].Add(b.Offset, var);
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
							if (instr.Operands.Count() > 0 && instr.Operands[0].IsVariable() && pointerValues.TryGetValue(instr.Operands[0].Var, pv))
							{
								if (instr.Func == Operation::Load)
								{
									instr.Func = 0;
									instr.Operands.Clear();
									instr.Operands.Add(replaceVars[pv.BaseVar->Id][pv.Offset].GetValue());
								}
								else if (instr.Func == Operation::Store)
								{
									instr.Func = 0;
									auto opValue = instr.Operands[1];
									instr.Operands.Clear();
									instr.Operands.Add(opValue);
									instr.LeftOperand = Operand(replaceVars[pv.BaseVar->Id][pv.Offset].GetValue());
								}
							}
						}
				}
				return rs;
			}
		};
		IntraProcOptimizer * CreateConstIndirectionRemovalOptimizer()
		{
			return new ConstIndirectionRemovalOptimizer();
		}
	}
}