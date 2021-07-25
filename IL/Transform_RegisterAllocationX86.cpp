#include "Optimization.h"
#include "CodeEmitter_x86.h"
#include "InterferenceAnalysis.h"
#include <float.h>
namespace Compiler
{
	namespace Intermediate
	{
		using namespace Compiler::x86;

		const static MemoryLocation EAX = MemoryLocation(MemoryLocationType::Register, 1);
		const static MemoryLocation ECX = MemoryLocation(MemoryLocationType::Register, 2);
		const static MemoryLocation EBX = MemoryLocation(MemoryLocationType::Register, 4);
		const static MemoryLocation ESI = MemoryLocation(MemoryLocationType::Register, 8);
		const static MemoryLocation EDI = MemoryLocation(MemoryLocationType::Register, 16);

		const int GeneralRegisterCount = 5;
		const int OccupiedRegisterState = 31;

		class RegisterAllocatorX86 : public IntraProcOptimizer
		{
		private:
			struct RankedVariable
			{
				double Rank;
				Variable * Var;
				RankedVariable()
				{}
				RankedVariable(Variable * var, double rank)
				{
					Var = var;
					Rank = rank;
				}
				bool operator < (const RankedVariable & var)
				{
					return Rank < var.Rank;
				}
			};
			void MeasureBlockFrequency(List<double> &frequencies, ControlFlowNode * node, double freq)
			{
				frequencies[node->Id] = freq;
				for (auto child : node->DomChildren)
				{
					if ((child->Exits[0] == node || child->Exits[1] == node) && (node->Exits[0] == child || node->Exits[1] == child))
					{
						MeasureBlockFrequency(frequencies, child, freq * 10.0);
					}
					else if (node->Exits[1] != 0)
						MeasureBlockFrequency(frequencies, child, freq * 0.5);
					else
						MeasureBlockFrequency(frequencies, child, freq);
				}
			}
			static int PickRegister(int regState)
			{
				if ((regState & EAX.Value) == 0)
					return EAX.Value;
				else if ((regState & ECX.Value) == 0)
					return ECX.Value;
				else if ((regState & EBX.Value) == 0)
					return EBX.Value;
				else if ((regState & ESI.Value) == 0)
					return ESI.Value;
				else if ((regState & EDI.Value) == 0)
					return EDI.Value;
				else
					return -1;
			}
		public:
			virtual ProgramOptimizationResult Optimize(RefPtr<ControlFlowGraph> program) override
			{
				ProgramOptimizationResult rs;
				rs.Program = program;
				rs.Changed = true;
				program->VariableSize = 0;
				for (int i = 0; i<program->Variables.Count(); i++)
				{
					auto var = program->Variables[i];
					var->Location.Type = MemoryLocationType::Stack;
					var->Location.Value = 0;
				}
				// Create short live ranges for contrained variables
				auto eaxedx = new Variable(L"%eax:edx", 8);
				eaxedx->Id = program->Variables.Count();
				program->Variables.Add(eaxedx);
				auto ecx = new Variable(L"%ecx", 4);
				ecx->Id = program->Variables.Count();
				program->Variables.Add(ecx);
				int paramCopyStartId = program->Variables.Count();
				for (int i = 0; i<program->ParameterCount; i++)
				{
					auto var = program->Variables[i];
					auto nVar = new Variable(*var);
					nVar->Id = program->Variables.Count();
					program->Variables.Add(nVar);
				}
				// insert temporary variable for mul and div and call and shl and shr, replace parameter uses with corresponding copy
				for (auto node : program->Nodes)
					for (auto instrNode = node->Code.begin(); instrNode!=node->Code.end(); ++instrNode)
					{
						auto &instr = instrNode.Current->Value;
						if (instr.LeftOperand.IsVariable() && instr.LeftOperand.Var->Id < program->ParameterCount)
							instr.LeftOperand.Var = program->Variables[instr.LeftOperand.Var->Id+paramCopyStartId].Ptr();
						for (auto & op : instr.Operands)
						{
							if (op.IsVariable() && op.Var->Id < program->ParameterCount)
								op.Var = program->Variables[op.Var->Id+paramCopyStartId].Ptr();
						}
						if (instr.Func == Operation::Mul || instr.Func == Operation::Div ||
							instr.Func == Operation::Mod || instr.Func == Operation::SInc ||
							instr.Func == Operation::Call)
						{
							if (instr.LeftOperand.IsVariable())
								instrNode.Current->InsertAfter(Instruction(instr.LeftOperand, 0, eaxedx));
							instr.LeftOperand.Var = eaxedx;
						}
						if (instr.Func == Operation::Lsh || instr.Func == Operation:: Rsh ||
							instr.Func == Operation::Div || instr.Func == Operation::Mod)
						{
							if (instr.Operands[1].IsVariable())
							{
								instrNode.Current->InsertBefore(Instruction(ecx, 0, instr.Operands[1]));
								instr.Operands[1].Var = ecx;
							}
						}
					}
				// insert copy for parameters
				auto startNode = program->Source->Exits[0];
				if (startNode)
				{
					for (int i = 0; i<program->ParameterCount; i++)
					{
						auto var = program->Variables[i].Ptr();
						auto nVar = program->Variables[i+paramCopyStartId].Ptr();
						startNode->Code.AddFirst(Instruction(Operand(nVar), 0, Operand(var)));
					}
				}
				// measure frequency
				List<double> frequencies;
				frequencies.SetSize(program->Nodes.Count());
				MeasureBlockFrequency(frequencies, program->Source, 1.0);
				List<RankedVariable> varRanks;
				varRanks.SetSize(program->Variables.Count());
				for (int i = 0; i<program->Variables.Count(); i++)
				{
					varRanks[i].Var = program->Variables[i].Ptr();
					if (i<program->ParameterCount || program->Variables[i]->Size == 0)
						varRanks[i].Rank = -DBL_MAX;
					else
						varRanks[i].Rank = 0;				
				}
				for (auto node : program->Nodes)
					for (auto instr : node->Code)
					{
						if (instr.Func == Operation::Lea)
						{
							varRanks[instr.Operands[0].Var->Id].Rank = -DBL_MAX;
							varRanks[instr.LeftOperand.Var->Id].Rank = -DBL_MAX;
							instr.LeftOperand.Var->Size = 0; // this variable will be optmized out by machine code generator
						}
						if (instr.LeftOperand.IsVariable())
							varRanks[instr.LeftOperand.Var->Id].Rank += frequencies[node->Id];
						for (auto op : instr.Operands)
							if (op.IsVariable())
								varRanks[op.Var->Id].Rank += frequencies[node->Id];
					}
				varRanks.Sort();
				// build interference graph
				auto interGraph = InterferenceAnalysis::BuildInterferenceGraph(program.Ptr());
				List<List<int>> neighbours;
				neighbours.SetSize(program->Variables.Count());
				for (int i = 1; i<program->Variables.Count(); i++)
				{
					for (int j = 0; j<i; j++)
					{
						if (interGraph.Interferes(i, j))
						{
							neighbours[i].Add(j);
							neighbours[j].Add(i);
						}
					}
				}
				// color by rank
				for (auto var : program->Variables)
				{
					var->Location.Type = MemoryLocationType::Stack;
					var->Location.Value = 0;
				}
				eaxedx->Location = EAX;
				ecx->Location = ECX;
				// color constrained variables first
				for (int pass = 0; pass < 2; pass++)
					for (int i = varRanks.Count()-1; i>=0; i--)
					{
						auto var = varRanks[i].Var;
						if (varRanks[i].Rank < 0.0)
							continue;
						if (var->Id < program->ParameterCount)
							continue;
						if (var->Location.Type == MemoryLocationType::Register)
							continue;
						int neighbourCount = neighbours[var->Id].Count();
						int regState = 0;
						for (auto varId : neighbours[var->Id])
						{
							if (program->Variables[varId]->Location.Type == MemoryLocationType::Register)
								regState |= program->Variables[varId]->Location.Value;
						}
						if (pass == 0)
						{
							if (neighbourCount >= GeneralRegisterCount)
							{
								int reg = PickRegister(regState);
								if (reg != -1)
								{
									var->Location.Type = MemoryLocationType::Register;
									var->Location.Value = reg;						
								}
							}
						}
						else
						{
							int reg = PickRegister(regState);
							if (reg != -1)
							{
								var->Location.Type = MemoryLocationType::Register;
								var->Location.Value = reg;						
							}
						}
					}
				// allocate stack position for spilled variables
				int offset = 8;
				for (int i = 0; i<program->ParameterCount; i++)
				{
					auto var = program->Variables[i];
					var->Location.Type = MemoryLocationType::Stack;
					var->Location.Value = offset;
					offset += var->Size;
				}
				offset = 0;
				for (int i = program->ParameterCount; i<program->Variables.Count(); i++)
				{
					auto var = program->Variables[i];
					if (var->Location.Type == MemoryLocationType::Stack)
					{
						program->VariableSize += var->Size;
						offset -= var->Size;
						var->Location.Value = offset;
						
					}
				}
				// translate register id to x86 register id
				for (auto var : program->Variables)
					if (var->Location.Type == MemoryLocationType::Register)
					{
						switch (var->Location.Value)
						{
						case 1:
							var->Location.Value = (int)Register::EAX;
							break;
						case 2:
							var->Location.Value = (int)Register::ECX;
							break;
						case 4:
							var->Location.Value = (int)Register::EBX;
							break;
						case 8:
							var->Location.Value = (int)Register::ESI;
							break;
						case 16:
							var->Location.Value = (int)Register::EDI;
							break;
						default:
							throw InvalidProgramException("Illegal register id.");
						}
					}
				// remove unnecessary parameter copies
				if (startNode)
				{
					auto instrNode = startNode->Code.begin();
					Dictionary<Variable*, Variable*> varMap;
					for (int i = 0; i<program->ParameterCount; i++)
					{
						auto & instr = instrNode.Current->Value;
						if (instr.LeftOperand.Var->Location.Type == MemoryLocationType::Stack)
						{
							varMap[instr.LeftOperand.Var] = instr.Operands[0].Var;
						}
						++instrNode;
					}
					if (varMap.Count())
					{
						for (auto node : program->Nodes)
							for (auto instr : node->Code)
							{
								Variable * nVar = 0;
								if (instr.LeftOperand.IsVariable() && varMap.TryGetValue(instr.LeftOperand.Var, nVar))
									instr.LeftOperand.Var = nVar;
								for (auto op : instr.Operands)
									if (op.IsVariable() && varMap.TryGetValue(op.Var, nVar))
										op.Var = nVar;
							}
					}
				}
				return rs;
			}
		};

		IntraProcOptimizer * CreateRegisterAllocator()
		{
			return new RegisterAllocatorX86();
		}
	}
}