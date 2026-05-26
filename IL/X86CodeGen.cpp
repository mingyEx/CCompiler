#include "X86CodeGen.h"
#include "CodeEmitter_x86.h"
#include <iterator>
#include <utility>
#include <unordered_map>
#include <vector>

namespace Compiler
{
	namespace Intermediate
	{
		using namespace Compiler::x86;

		struct LabelPoint
		{
			int LineId;
			int* Position;
			LabelPoint()
			{}
			LabelPoint(int lineId, int* pos)
				: LineId(lineId), Position(pos)
			{}
		};

		class X86CodeGeneratorImpl : public X86CodeGenerator
		{
		private:
			std::unordered_map<Variable *, int> ptr_var_offset;
			Function_x86 * curFunc;
			int labelGenerator;
		private:
			x86::Instruction & Emit(x86::Instruction::InstructionName instrName, const x86::Operand & op1, const x86::Operand & op2)
			{
				if (instrName == x86::Instruction::MOV && op1 == op2)
					throw "st";
				curFunc->Code.push_back(x86::Instruction(instrName, op1, op2));
				curFunc->Code.back().Label = labelGenerator++;
				return curFunc->Code.back();
			}

			x86::Instruction &  Emit(x86::Instruction::InstructionName instrName, const x86::Operand & op1)
			{
				curFunc->Code.push_back(x86::Instruction(instrName, op1));
				curFunc->Code.back().Label = labelGenerator++;
				return curFunc->Code.back();
			}

			x86::Instruction &  Emit(x86::Instruction::InstructionName instrName)
			{
				curFunc->Code.push_back(x86::Instruction(instrName));
				curFunc->Code.back().Label = labelGenerator++;
				return curFunc->Code.back();
			}

			int EmitFloatConst(float val)
			{
				FloatConstant f;
				f.Type = FloatType::Float;
				f.FloatValue = val;
				curFunc->FloatConsts.push_back(f);
				return static_cast<int>(curFunc->FloatConsts.size()) - 1;
			}

			int EmitFloatConst(double val)
			{
				FloatConstant f;
				f.Type = FloatType::Double;
				f.DoubleValue = val;
				curFunc->FloatConsts.push_back(f);
				return static_cast<int>(curFunc->FloatConsts.size()) - 1;
			}

			void EmitLoad(Register dest, int offset) // dest = [EBP+offset]
			{
				if (offset>=CHAR_MIN && offset <= CHAR_MAX)
				{
					Emit(x86::Instruction::MOV, x86::Operand(dest), x86::Operand::FromMemory(MemoryOperandType::Int32, Register::EBP, offset));
				}
				else
				{
					Emit(x86::Instruction::MOV, x86::Operand(dest), x86::Operand::FromMemory(MemoryOperandType::Int32, Register::EBP, offset));
				}
			}
			void EmitStore(Register source, int offset) // [EBP+offset] = source
			{
				if (offset>=CHAR_MIN && offset <= CHAR_MAX)
				{
					Emit(x86::Instruction::MOV, x86::Operand::FromMemory(MemoryOperandType::Int32, Register::EBP, offset), x86::Operand(source));
				}
				else
				{
					Emit(x86::Instruction::MOV, x86::Operand::FromMemory(MemoryOperandType::Int32, Register::EBP, offset), x86::Operand(source));
				}
			}

			x86::Operand LoadOp(Operand & op, Register reg = Register::EDX)
			{
				if (op.IsVariable())
				{
					if (op.Var->Location.Type == MemoryLocationType::Register)
						return x86::Operand((Register)op.Var->Location.Value);
					else if (op.Var->Type == DataType::Int)
					{
						EmitLoad(reg, op.Var->Location.Value);
						return x86::Operand(reg);
					}
					else if (op.Var->Type == DataType::Double)
					{
						Emit(x86::Instruction::FLD, x86::Operand::FromMemory(MemoryOperandType::Float64, Register::EBP, op.Var->Location.Value));
					}
					else if (op.Var->Type == DataType::Float)
					{
						Emit(x86::Instruction::FLD, x86::Operand::FromMemory(MemoryOperandType::Float32, Register::EBP, op.Var->Location.Value));
					}
				}
				else 
				{
					if (op.Type == OperandType::ConstInt32 ||
						op.Type == OperandType::ConstInt16 ||
						op.Type == OperandType::ConstInt8)
					{
						Emit(x86::Instruction::MOV, x86::Operand(reg), x86::Operand(unsigned int(op.IntValue)));
						return x86::Operand(reg);
					}
					else if (op.Type == OperandType::ConstDouble)
					{
						int constId = EmitFloatConst(op.DoubleValue);
						Emit(x86::Instruction::FLD, x86::Operand::FromMemory(MemoryOperandType::Float64, constId));
					}
					else if (op.Type == OperandType::ConstFloat)
					{
						int constId = EmitFloatConst(op.FloatValue);
						Emit(x86::Instruction::FLD, x86::Operand::FromMemory(MemoryOperandType::Float32, constId));
					}
				}
				return x86::Operand(Register::None);
			}

			x86::Operand GetOp(Operand & op)
			{
				if (op.IsVariable())
				{
					MemoryOperandType memType;
					if (op.Var->Location.Type == MemoryLocationType::Register)
						return x86::Operand((Register)op.Var->Location.Value);
					else if (op.Var->Type == DataType::Int)
						memType = MemoryOperandType::Int32;
					else if (op.Var->Type == DataType::Float)
						memType = MemoryOperandType::Float32;
					else if (op.Var->Type == DataType::Double)
						memType = MemoryOperandType::Float64; 
					else
						throw InvalidProgramException(L"Unsupported operand data type.");
					return x86::Operand::FromMemory(memType, Register::EBP, op.Var->Location.Value);
				}
				if (op.Type == OperandType::ConstDouble || op.Type == OperandType::ConstFloat)
				{
					return LoadOp(op);
				}
				else if (op.Type == OperandType::ConstInt16 || op.Type == OperandType::ConstInt8 ||
					op.Type == OperandType::ConstInt32)
				{
					return x86::Operand((unsigned int)op.IntValue);
				}
				return x86::Operand(Register::None);
			}

			static bool IsFloatingDataType(DataType type)
			{
				return type == DataType::Float || type == DataType::Double;
			}

			x86::Operand GetFloatMemoryOp(Operand & op)
			{
				if (!op.IsVariable())
					throw InvalidProgramException(L"Floating-point result must be stored in a variable.");
				if (op.Var->Location.Type == MemoryLocationType::Register)
					throw InvalidProgramException(L"Floating-point variables cannot use general registers.");
				if (op.Var->Type == DataType::Double)
					return x86::Operand::FromMemory(MemoryOperandType::Float64, Register::EBP, op.Var->Location.Value);
				if (op.Var->Type == DataType::Float)
					return x86::Operand::FromMemory(MemoryOperandType::Float32, Register::EBP, op.Var->Location.Value);
				throw InvalidProgramException(L"Unsupported floating-point operand.");
			}

			void LoadFloatOp(Operand & op)
			{
				if (op.IsVariable())
				{
					if (op.Var->Location.Type == MemoryLocationType::Register)
						throw InvalidProgramException(L"Floating-point variables cannot use general registers.");
					if (op.Var->Type == DataType::Double)
					{
						Emit(x86::Instruction::FLD, x86::Operand::FromMemory(MemoryOperandType::Float64, Register::EBP, op.Var->Location.Value));
						return;
					}
					if (op.Var->Type == DataType::Float)
					{
						Emit(x86::Instruction::FLD, x86::Operand::FromMemory(MemoryOperandType::Float32, Register::EBP, op.Var->Location.Value));
						return;
					}
				}
				else if (op.Type == OperandType::ConstDouble)
				{
					int constId = EmitFloatConst(op.DoubleValue);
					Emit(x86::Instruction::FLD, x86::Operand::FromMemory(MemoryOperandType::Float64, constId));
					return;
				}
				else if (op.Type == OperandType::ConstFloat)
				{
					int constId = EmitFloatConst(op.FloatValue);
					Emit(x86::Instruction::FLD, x86::Operand::FromMemory(MemoryOperandType::Float32, constId));
					return;
				}
				throw InvalidProgramException(L"Unsupported floating-point operand.");
			}

			void LoadIntAsFloat(Operand & op)
			{
				auto sourceOp = GetOp(op);
				if (sourceOp.Type == x86::OperandType::Memory)
				{
					if (sourceOp.MemoryType != MemoryOperandType::Int16 &&
						sourceOp.MemoryType != MemoryOperandType::Int32 &&
						sourceOp.MemoryType != MemoryOperandType::Int64)
						throw InvalidProgramException(L"Unsupported integer conversion operand.");
					Emit(x86::Instruction::FILD, sourceOp);
					return;
				}
				Emit(x86::Instruction::PUSH, sourceOp);
				Emit(x86::Instruction::FILD, x86::Operand::FromMemory(MemoryOperandType::Int32, Register::ESP, 0));
				Emit(x86::Instruction::ADD, x86::Operand(Register::ESP), x86::Operand((unsigned int)4));
			}

			void StoreFloatResult(Operand & op)
			{
				Emit(x86::Instruction::FSTP, GetFloatMemoryOp(op));
			}

			void EmitFloatBinary(Intermediate::Instruction & instr, x86::Instruction::InstructionName instrName)
			{
				if (IsFloatingDataType(instr.Operands[0].GetDataType()))
					LoadFloatOp(instr.Operands[0]);
				else
					LoadIntAsFloat(instr.Operands[0]);
				if (IsFloatingDataType(instr.Operands[1].GetDataType()))
					LoadFloatOp(instr.Operands[1]);
				else
					LoadIntAsFloat(instr.Operands[1]);
				Emit(instrName);
				StoreFloatResult(instr.LeftOperand);
			}

			void EmitI2D(Intermediate::Instruction & instr)
			{
				LoadIntAsFloat(instr.Operands[0]);
				StoreFloatResult(instr.LeftOperand);
			}

			void EmitMov(Intermediate::Instruction & instr)
			{
				auto leftType = instr.LeftOperand.GetDataType();
				auto rightType = instr.Operands[0].GetDataType();
				if (IsFloatingDataType(leftType))
				{
					if (IsFloatingDataType(rightType))
						LoadFloatOp(instr.Operands[0]);
					else
						LoadIntAsFloat(instr.Operands[0]);
					StoreFloatResult(instr.LeftOperand);
					return;
				}
				if (IsFloatingDataType(rightType))
					throw InvalidProgramException(L"Unsupported floating-point to integer move.");

				auto op1 = GetOp(instr.LeftOperand);
				if (!instr.Operands[0].IsIntegral())
				{
					Register tmpReg = Register::EDX;
					if (op1.Type == x86::OperandType::Register)
						tmpReg = op1.Reg;
					auto op2 = LoadOp(instr.Operands[0], tmpReg);
					if (op1 != op2)
						Emit(x86::Instruction::MOV, op1, op2);
				}
				else
				{
					auto op2 = GetOp(instr.Operands[0]);
					if (op1 != op2)
						Emit(x86::Instruction::MOV, op1, op2);
				}
			}

			void StoreResult(Register resultReg, Operand op)
			{
				auto op1 = GetOp(op);
				auto op2 = x86::Operand(resultReg);
				if (op1 != op2)
					Emit(x86::Instruction::MOV, op1, op2);
			}

			bool IsOpAtReg(Operand & op, Register reg)
			{
				if (!op.IsVariable())
					return false;
				if (op.Var->Location.Type == MemoryLocationType::Register &&
					op.Var->Location.Value == (int)reg)
					return true;
				return false;
			}

			void EmitBinary(Intermediate::Instruction & instr, x86::Instruction::InstructionName instrName)
			{
				auto leftOp = GetOp(instr.LeftOperand);
				x86::Operand op0;
				if (instr.LeftOperand.Var->Location.Type == MemoryLocationType::Register)
					op0 = GetOp(instr.Operands[0]);
				else
					op0 = LoadOp(instr.Operands[0]);
				
				if (leftOp.Type == x86::OperandType::Register)
				{
					if (IsOpAtReg(instr.Operands[1], leftOp.Reg))
					{
						Emit(x86::Instruction::XCHG, Register::EDX, leftOp);
						if (op0.Type == x86::OperandType::Register && op0.Value == (int)Register::EDX)
							op0.Value = leftOp.Value;
						if (op0 != leftOp)
							Emit(x86::Instruction::MOV, leftOp, op0);
						Emit(instrName, leftOp, Register::EDX);
					}
					else
					{
						if (leftOp != op0)
							Emit(x86::Instruction::MOV, leftOp, op0);
						Emit(instrName, leftOp, GetOp(instr.Operands[1]));
					}
				}
				else
				{
					if (leftOp != op0)
						Emit(x86::Instruction::MOV, leftOp, op0);
					Emit(instrName, leftOp, LoadOp(instr.Operands[1]));
				}
			}

			void EmitAdd(Intermediate::Instruction & instr)
			{
				if (instr.Operands[1].IsIntegral() && instr.Operands[1].IntValue == 1 && instr.Operands[0].IsVariable() && instr.LeftOperand.Var == instr.Operands[0].Var)
					Emit(x86::Instruction::INC, GetOp(instr.Operands[0]));
				else
					EmitBinary(instr, x86::Instruction::ADD);
			}

			void EmitSub(Intermediate::Instruction & instr)
			{
				if (instr.Operands[1].IsIntegral() && instr.Operands[1].IntValue == 1 && instr.Operands[0].IsVariable() && instr.LeftOperand.Var == instr.Operands[0].Var)
					Emit(x86::Instruction::DEC, GetOp(instr.Operands[0]));
				else
					EmitBinary(instr, x86::Instruction::SUB);
			}

			void EmitLsh(Intermediate::Instruction & instr)
			{
				EmitBinary(instr, x86::Instruction::SHL);
			}

			void EmitRsh(Intermediate::Instruction & instr)
			{
				EmitBinary(instr, x86::Instruction::SHR);
			}

			void EmitMul(Intermediate::Instruction & instr)
			{
				if (instr.LeftOperand.Var->Location.Type != MemoryLocationType::Register ||
					instr.LeftOperand.Var->Location.Value != (int)Register::EAX)
					throw InvalidProgramException(L"invalid register allocation");
				if (instr.Operands[1].IsVariable() && instr.Operands[1].Var->Location.Type == MemoryLocationType::Register
					&& instr.Operands[1].Var->Location.Value == (int)Register::EAX)
				{
					Emit(x86::Instruction::IMUL, GetOp(instr.Operands[0]));
				}
				else
				{
					auto op0 = GetOp(instr.Operands[0]);
					if (op0 != x86::Operand(Register::EAX))
						Emit(x86::Instruction::MOV, x86::Operand(Register::EAX), op0);
					if (instr.Operands[1].IsIntegral())
						Emit(x86::Instruction::IMUL, LoadOp(instr.Operands[1]));
					else
						Emit(x86::Instruction::IMUL, GetOp(instr.Operands[1]));
				}
			}

			void EmitDiv(Intermediate::Instruction & instr)
			{
				if (instr.LeftOperand.Var->Location.Type != MemoryLocationType::Register ||
					instr.LeftOperand.Var->Location.Value != (int)Register::EAX)
					throw InvalidProgramException(L"invalid register allocation");
				auto op1 = GetOp(instr.Operands[1]);
				bool shouldPop = false;
				if (!instr.Operands[1].IsVariable() ||  instr.Operands[1].IsVariable() && instr.Operands[1].Var->Location.Type == MemoryLocationType::Register
					&& instr.Operands[1].Var->Location.Value == (int)Register::EAX)
				{
					Emit(x86::Instruction::PUSH, op1);
					op1 = x86::Operand::FromMemory(MemoryOperandType::Int32, Register::ESP, 0);
					shouldPop = true;
				}
				auto op0 = GetOp(instr.Operands[0]);
				if (op0 != x86::Operand(Register::EAX))
					Emit(x86::Instruction::MOV, x86::Operand(Register::EAX), op0);
				Emit(x86::Instruction::CDQ);
				Emit(x86::Instruction::IDIV, op1);
				if (shouldPop)
					Emit(x86::Instruction::ADD, Register::ESP, (unsigned int)4);
			}

			void EmitMod(Intermediate::Instruction & instr)
			{
				if (instr.LeftOperand.Var->Location.Type != MemoryLocationType::Register ||
					instr.LeftOperand.Var->Location.Value != (int)Register::EAX)
					throw InvalidProgramException(L"invalid register allocation");
				auto op1 = GetOp(instr.Operands[1]);
				bool shouldPop = false;
				if (!instr.Operands[1].IsVariable() ||  instr.Operands[1].IsVariable() && instr.Operands[1].Var->Location.Type == MemoryLocationType::Register
					&& instr.Operands[1].Var->Location.Value == (int)Register::EAX)
				{
					Emit(x86::Instruction::PUSH, op1);
					op1 = x86::Operand::FromMemory(MemoryOperandType::Int32, Register::ESP, 0);
					shouldPop = true;
				}
				auto op0 = GetOp(instr.Operands[0]);
				if (op0 != x86::Operand(Register::EAX))
					Emit(x86::Instruction::MOV, x86::Operand(Register::EAX), op0);
				Emit(x86::Instruction::CDQ);
				Emit(x86::Instruction::IDIV, op1);
				Emit(x86::Instruction::MOV, x86::Operand(Register::EAX), x86::Operand(Register::EDX));

				if (shouldPop)
					Emit(x86::Instruction::ADD, Register::ESP, (unsigned int)4);
			}

			void EmitX86Compare(Intermediate::Instruction & instr)
			{
				if (instr.Operands[0].GetDataType() == DataType::Float ||
					instr.Operands[0].GetDataType() == DataType::Double)
					Emit(x86::Instruction::FCOMPP, LoadOp(instr.Operands[0]), GetOp(instr.Operands[1]));
				else
				{
					if (!instr.Operands[1].IsIntegral())
						Emit(x86::Instruction::CMP, LoadOp(instr.Operands[0]), GetOp(instr.Operands[1]));
					else
						Emit(x86::Instruction::CMP, GetOp(instr.Operands[0]), GetOp(instr.Operands[1]));
				}
			}

			void EmitCompare(Intermediate::Instruction & instr)
			{
				EmitX86Compare(instr);
				auto targetOp = GetOp(instr.LeftOperand);
				Emit(x86::Instruction::MOV, targetOp, x86::Operand(0u));
				targetOp.SizeBits = 8;
				if (instr.Func == Operation::Less)
					Emit(x86::Instruction::SETL, targetOp);
				else if (instr.Func == Operation::Greater)
					Emit(x86::Instruction::SETG, targetOp);
				else if (instr.Func == Operation::Geq)
					Emit(x86::Instruction::SETGE, targetOp);
				else if (instr.Func == Operation::Leq)
					Emit(x86::Instruction::SETLE, targetOp);
				else if (instr.Func == Operation::Eql)
					Emit(x86::Instruction::SETE, targetOp);
				else
					Emit(x86::Instruction::SETNE, targetOp);
			}

			void EmitAnd(Intermediate::Instruction & instr)
			{
				auto op1 = GetOp(instr.Operands[0]);
				Emit(x86::Instruction::TEST, op1, x86::Operand(0xFFFFFFFFu));
				Emit(x86::Instruction::XOR, x86::Operand(Register::EDX), x86::Operand(Register::EDX));
				Emit(x86::Instruction::SETNZ, x86::Operand(Register::EDX, 8));
				auto op2 = GetOp(instr.Operands[1]);
				Emit(x86::Instruction::TEST, op2, x86::Operand(0xFFFFFFFFu));
				auto leftOp = GetOp(instr.LeftOperand);
				auto flagOp = leftOp;
				flagOp.SizeBits = 8;
				Emit(x86::Instruction::SETNZ, flagOp);
				Emit(x86::Instruction::AND, GetOp(instr.LeftOperand), x86::Operand(Register::EDX));
			}

			void EmitOr(Intermediate::Instruction & instr)
			{
				auto op1 = GetOp(instr.Operands[0]);
				Emit(x86::Instruction::TEST, op1, x86::Operand(0xFFFFFFFFu));
				Emit(x86::Instruction::XOR, x86::Operand(Register::EDX), x86::Operand(Register::EDX));
				Emit(x86::Instruction::SETNZ, x86::Operand(Register::EDX, 8));
				auto op2 = GetOp(instr.Operands[1]);
				Emit(x86::Instruction::TEST, op2, x86::Operand(0xFFFFFFFFu));
				auto leftOp = GetOp(instr.LeftOperand);
				Emit(x86::Instruction::MOV, leftOp, x86::Operand(0u));
				auto flagOp = leftOp;
				flagOp.SizeBits = 8;
				Emit(x86::Instruction::SETNZ, flagOp);
				Emit(x86::Instruction::OR, GetOp(instr.LeftOperand), x86::Operand(Register::EDX));
			}

			void EmitNot(Intermediate::Instruction & instr)
			{
				auto op1 = LoadOp(instr.Operands[0]);
				Emit(x86::Instruction::TEST, op1, x86::Operand(0xFFFFFFFFu));
				Emit(x86::Instruction::XOR, x86::Operand(Register::EDX), x86::Operand(Register::EDX));
				Emit(x86::Instruction::SETZ, x86::Operand(Register::EDX, 8));
				StoreResult(Register::EDX, instr.LeftOperand);
			}

			void EmitIntUnary(Intermediate::Instruction & instr, x86::Instruction::InstructionName instrName)
			{
				if (instr.Operands[0].GetDataType() != DataType::Int)
					throw InvalidProgramException(L"Unsupported unary operand type.");

				auto sourceOp = GetOp(instr.Operands[0]);
				if (sourceOp != x86::Operand(Register::EDX))
					Emit(x86::Instruction::MOV, x86::Operand(Register::EDX), sourceOp);
				Emit(instrName, x86::Operand(Register::EDX));
				StoreResult(Register::EDX, instr.LeftOperand);
			}

			void EmitNeg(Intermediate::Instruction & instr)
			{
				EmitIntUnary(instr, x86::Instruction::NEG);
			}

			void EmitBitAnd(Intermediate::Instruction & instr)
			{
				EmitBinary(instr, x86::Instruction::AND);
			}

			void EmitBitOr(Intermediate::Instruction & instr)
			{
				EmitBinary(instr, x86::Instruction::OR);
			}

			void EmitBitXor(Intermediate::Instruction & instr)
			{
				EmitBinary(instr, x86::Instruction::XOR);
			}

			void EmitBitNot(Intermediate::Instruction & instr)
			{
				EmitIntUnary(instr, x86::Instruction::NOT);
			}

			void EmitCall(Intermediate::Instruction & instr)
			{
				// save register
				//Emit(x86::Instruction::PUSH, x86::Operand(Register::EAX));
				Emit(x86::Instruction::PUSH, x86::Operand(Register::ECX));
				//Emit(x86::Instruction::PUSH, x86::Operand(Register::EDX));
				// push parameters
				for (size_t i = instr.Operands.size(); i-- > 1;)
				{
					Emit(x86::Instruction::PUSH, GetOp(instr.Operands[i]));
				}
				Emit(x86::Instruction::CALL, x86::Operand((unsigned int)instr.Operands[0].IntValue));
				// restore register
				//Emit(x86::Instruction::POP, x86::Operand(Register::EDX));
				Emit(x86::Instruction::POP, x86::Operand(Register::ECX));
				//Emit(x86::Instruction::POP, x86::Operand(Register::EAX));
			}

			void EmitRet(Intermediate::Instruction & instr)
			{
				if (instr.Operands.size() == 2)
				{
					if (IsFloatingDataType(instr.Operands[1].GetDataType()))
					{
						LoadFloatOp(instr.Operands[1]);
					}
					else
					{
						auto op = GetOp(instr.Operands[1]);
						if (op != x86::Operand(Register::EAX))
							Emit(x86::Instruction::MOV, x86::Operand(Register::EAX), op);
					}
				}
				Emit(x86::Instruction::POP, x86::Operand(Register::EBX));
				Emit(x86::Instruction::POP, x86::Operand(Register::EDI));
				Emit(x86::Instruction::POP, x86::Operand(Register::ESI));
				Emit(x86::Instruction::MOV, x86::Operand(Register::ESP), x86::Operand(Register::EBP));
				Emit(x86::Instruction::POP, x86::Operand(Register::EBP));

				Emit(x86::Instruction::RET, x86::Operand((unsigned short)instr.Operands[0].IntValue));
			}

			void EmitJump(Intermediate::Instruction & instr, std::vector<LabelPoint> & labels)
			{
				auto & ins = Emit(x86::Instruction::JMP, x86::Operand((unsigned int)instr.Operands[0].IntValue));
				labels.push_back(LabelPoint(instr.Operands[0].IntValue, (int*)&ins.Op1.Value));
			}

			void EmitBranch(Intermediate::Instruction & instr, std::vector<LabelPoint> & labels)
			{
				int branchType = instr.Operands[2].IntValue;
				if (branchType < 2)
				{
					auto op1 = LoadOp(instr.Operands[0]);
					Emit(x86::Instruction::TEST, op1, x86::Operand(0xFFFFFFFFu));
				}
				x86::Instruction * ins = 0;
				if (branchType == 0) // jtrue
					ins = &Emit(x86::Instruction::JNZ, x86::Operand((unsigned int)instr.Operands[1].IntValue));
				else if (branchType == 1) // jfalse
					ins = &Emit(x86::Instruction::JZ, x86::Operand((unsigned int)instr.Operands[1].IntValue));
				else if (branchType == 2) // jl
					ins = &Emit(x86::Instruction::JL, x86::Operand((unsigned int)instr.Operands[1].IntValue));
				else if (branchType == 3) // jge
					ins = &Emit(x86::Instruction::JGE, x86::Operand((unsigned int)instr.Operands[1].IntValue));
				else if (branchType == 4) // jg
					ins = &Emit(x86::Instruction::JG, x86::Operand((unsigned int)instr.Operands[1].IntValue));
				else if (branchType == 5) // jle
					ins = &Emit(x86::Instruction::JLE, x86::Operand((unsigned int)instr.Operands[1].IntValue));
				else if (branchType == 6) // je
					ins = &Emit(x86::Instruction::JE, x86::Operand((unsigned int)instr.Operands[1].IntValue));
				else if (branchType == 7) // jne
					ins = &Emit(x86::Instruction::JNE, x86::Operand((unsigned int)instr.Operands[1].IntValue));
				else
					throw InvalidProgramException("unkown branch type");
				labels.push_back(LabelPoint(instr.Operands[1].IntValue, (int*)&ins->Op1.Value));
			}

			void EmitLoad(Intermediate::Instruction & instr)
			{
				MemoryOperandType memType;
				bool isInt = true;
				auto tempReg = x86::Operand(Register::EDX);
				if (!instr.LeftOperand.IsVariable())
					throw InvalidProgramException("load must have a variable left operand");
				if (instr.LeftOperand.Var->Location.Type == MemoryLocationType::Register)
					tempReg = x86::Operand((Register)instr.LeftOperand.Var->Location.Value);
				switch (instr.Operands[1].IntValue)
				{
				case 1:
					memType = MemoryOperandType::Int32;
					break;
				case 2:
					Emit(x86::Instruction::XOR, tempReg, tempReg);
					memType = MemoryOperandType::Int16;
					break;
				case 3:
					memType = MemoryOperandType::Float64;
					isInt = false;
					break;
				default:
					throw InvalidProgramException(L"Unsupported memory type.");
					break;
				}
				auto addr = LoadOp(instr.Operands[0]);
				if (addr.Type != x86::OperandType::Register)
				{
					throw InvalidProgramException(L"address operand should be in register.");
				}
				if (isInt)
				{
					Emit(x86::Instruction::MOV, tempReg, x86::Operand::FromMemory(memType, addr.Reg, 0));
					if (tempReg == x86::Operand(Register::EDX))
						Emit(x86::Instruction::MOV, GetOp(instr.LeftOperand), x86::Operand(Register::EDX));
				}
				else
				{
					Emit(x86::Instruction::FLD, x86::Operand::FromMemory(memType, addr.Reg, 0));
					Emit(x86::Instruction::FSTP, GetOp(instr.LeftOperand));
				}
			}

			void EmitStore(Intermediate::Instruction & instr)
			{
				MemoryOperandType memType;
				bool isInt = true;
				switch (instr.Operands[2].IntValue)
				{
				case 1:
					memType = MemoryOperandType::Int32;
					break;
				case 2:
					Emit(x86::Instruction::XOR, x86::Operand(Register::EDX), x86::Operand(Register::EDX));
					memType = MemoryOperandType::Int16;
					break;
				case 3:
					memType = MemoryOperandType::Float64;
					isInt = false;
					break;
				default:
					throw InvalidProgramException(L"Unsupported memory type.");
					break;
				}
				auto addr = LoadOp(instr.Operands[0]);
				if (addr.Type != x86::OperandType::Register)
				{
					throw InvalidProgramException(L"address operand should be in register.");
				}
				if (isInt)
				{
					auto value = LoadOp(instr.Operands[1], Register::EDX);
					Emit(x86::Instruction::MOV, x86::Operand::FromMemory(memType, addr.Reg, 0), value);
				}
				else
				{
					auto value = LoadOp(instr.Operands[1]);
					Emit(x86::Instruction::FSTP, x86::Operand::FromMemory(memType, addr.Reg, 0));
				}
			}

			void EmitLea(Intermediate::Instruction & instr)
			{
				ptr_var_offset[instr.LeftOperand.Var] = instr.Operands[0].Var->Location.Value;
				auto leftOp = GetOp(instr.LeftOperand);
				if (leftOp.Type == x86::OperandType::Register)
					Emit(x86::Instruction::LEA, leftOp, x86::Operand::FromMemory(MemoryOperandType::Int32, Register::EBP, instr.Operands[0].Var->Location.Value));
				else
				{
					Emit(x86::Instruction::LEA, Register::EDX, x86::Operand::FromMemory(MemoryOperandType::Int32, Register::EBP, instr.Operands[0].Var->Location.Value));
					StoreResult(Register::EDX, instr.LeftOperand);
				}
			}

			void EmitSInc(Intermediate::Instruction & instr)
			{
				if (instr.LeftOperand.Var->Location.Value != (int)Register::EAX)
					throw InvalidProgramException("invalid register allocation");
				auto op1 = LoadOp(instr.Operands[1]);
				int displacement = 0;
				int scale = 1;
				Register base = Register::EBP;
				if (instr.Operands[2].IsIntegral())
				{
					scale = instr.Operands[2].IntValue;
				}
				else
				{
					// compute eax = op1 * scale
					if (op1 != x86::Operand(Register::EAX))
						Emit(x86::Instruction::MOV, x86::Operand(Register::EAX), op1);
					Emit(x86::Instruction::MUL, LoadOp(instr.Operands[2]));
					Emit(x86::Instruction::ADD, x86::Operand(Register::EAX), GetOp(instr.Operands[0]));
					op1 = x86::Operand(Register::EAX);
				}
				if (auto iter = ptr_var_offset.find(instr.Operands[0].Var); iter != ptr_var_offset.end())
				{
					displacement = iter->second;
				}
				else
				{
					base = Register::EDX;
					Emit(x86::Instruction::MOV, Register::EDX, GetOp(instr.Operands[0]));
				}
				
				Emit(x86::Instruction::LEA, Register::EAX, x86::Operand::FromMemory(MemoryOperandType::Int32,
					base, (Register)op1.Reg, scale, displacement));
			}

			Function_x86 GenerateFunction(Function & function)
			{
				Function_x86 rs;
				std::vector<LabelPoint> labels;
				std::vector<int> labelMap;
				rs.Name = function.Name;
				curFunc = &rs;
				labelMap.reserve(function.Instructions.Count());
				Emit(x86::Instruction::PUSH, x86::Operand(Register::EBP));
				Emit(x86::Instruction::MOV, x86::Operand(Register::EBP), x86::Operand(Register::ESP));
				if (function.VariableSize > 0)
					Emit(x86::Instruction::SUB, x86::Operand(Register::ESP), x86::Operand((unsigned int)function.VariableSize));
				Emit(x86::Instruction::PUSH, x86::Operand(Register::ESI));
				Emit(x86::Instruction::PUSH, x86::Operand(Register::EDI));
				Emit(x86::Instruction::PUSH, x86::Operand(Register::EBX));
				for (auto & instr : function.Instructions)
				{
					labelMap.push_back(labelGenerator);
					if (instr.Func == 0)
						EmitMov(instr);
					else if (instr.Func == Operation::Add)
						EmitAdd(instr);
					else if (instr.Func == Operation::FAdd)
						EmitFloatBinary(instr, x86::Instruction::FADDP);
					else if (instr.Func == Operation::Sub)
						EmitSub(instr);
					else if (instr.Func == Operation::FSub)
						EmitFloatBinary(instr, x86::Instruction::FSUBP);
					else if (instr.Func == Operation::Mul)
						EmitMul(instr);
					else if (instr.Func == Operation::FMul)
						EmitFloatBinary(instr, x86::Instruction::FMULP);
					else if (instr.Func == Operation::Div)
						EmitDiv(instr);
					else if (instr.Func == Operation::FDiv)
						EmitFloatBinary(instr, x86::Instruction::FDIVP);
					else if (instr.Func == Operation::Mod)
						EmitMod(instr);
					else if (instr.Func == Operation::I2D)
						EmitI2D(instr);
					else if (instr.Func == Operation::Lsh)
						EmitLsh(instr);
					else if (instr.Func == Operation::Rsh)
						EmitRsh(instr);
					else if (instr.Func == Operation::Less || instr.Func == Operation::Greater ||
						instr.Func == Operation::Leq || instr.Func == Operation::Geq ||
						instr.Func == Operation::Eql || instr.Func == Operation::Neq)
						EmitCompare(instr);
					else if (instr.Func == Operation::Compare)
						EmitX86Compare(instr);
					else if (instr.Func == Operation::And)
						EmitAnd(instr);
					else if (instr.Func == Operation::Or)
						EmitOr(instr);
					else if (instr.Func == Operation::Neg)
						EmitNeg(instr);
					else if (instr.Func == Operation::Not)
						EmitNot(instr);
					else if (instr.Func == Operation::BitAnd)
						EmitBitAnd(instr);
					else if (instr.Func == Operation::BitOr)
						EmitBitOr(instr);
					else if (instr.Func == Operation::BitXor)
						EmitBitXor(instr);
					else if (instr.Func == Operation::BitNot)
						EmitBitNot(instr);
					else if (instr.Func == Operation::Jump)
						EmitJump(instr, labels);
					else if (instr.Func == Operation::Branch)
						EmitBranch(instr, labels);
					else if (instr.Func == Operation::Call)
						EmitCall(instr);
					else if (instr.Func == Operation::Ret)
						EmitRet(instr);
					else if (instr.Func == Operation::Load)
						EmitLoad(instr);
					else if (instr.Func == Operation::Store)
						EmitStore(instr);
					else if (instr.Func == Operation::SInc)
						EmitSInc(instr);
					else if (instr.Func == Operation::Lea)
						EmitLea(instr);
					else
						throw InvalidProgramException(L"Unsupported x86 operation.");
				}
				// fill in jump labels
				for (auto label : labels)
				{
					*label.Position = labelMap[label.LineId];
				}
				return rs;
			}
		private:
			x86::Instruction::InstructionName GetOppositeJcc(x86::Instruction::InstructionName name)
			{
				switch (name)
				{
				case x86::Instruction::JA:
					return x86::Instruction::JBE;
					break;
				case x86::Instruction::JAE:
					return x86::Instruction::JB;
					break;
				case x86::Instruction::JB:
					return x86::Instruction::JAE;
					break;
				case x86::Instruction::JBE:
					return x86::Instruction::JA;
					break;
				case x86::Instruction::JC:
					return x86::Instruction::JNC;
					break;
				case x86::Instruction::JCXZ:
					return x86::Instruction::UNKNOWN;
					break;
				case x86::Instruction::JE:
					return x86::Instruction::JNE;
					break;
				case x86::Instruction::JECXZ:
					return x86::Instruction::UNKNOWN;
					break;
				case x86::Instruction::JG:
					return x86::Instruction::JLE;
					break;
				case x86::Instruction::JGE:
					return x86::Instruction::JL;
					break;
				case x86::Instruction::JL:
					return x86::Instruction::JGE;
					break;
				case x86::Instruction::JLE:
					return x86::Instruction::JG;
					break;
				case x86::Instruction::JMP:
					return x86::Instruction::UNKNOWN;
					break;
				case x86::Instruction::JNA:
					return x86::Instruction::JA;
					break;
				case x86::Instruction::JNAE:
					return x86::Instruction::JAE;
					break;
				case x86::Instruction::JNB:
					return x86::Instruction::JB;
					break;
				case x86::Instruction::JNBE:
					return x86::Instruction::JBE;
					break;
				case x86::Instruction::JNC:
					return x86::Instruction::JC;
					break;
				case x86::Instruction::JNE:
					return x86::Instruction::JE;
					break;
				case x86::Instruction::JNG:
					return x86::Instruction::JG;
					break;
				case x86::Instruction::JNGE:
					return x86::Instruction::JGE;
					break;
				case x86::Instruction::JNL:
					return x86::Instruction::JL;
					break;
				case x86::Instruction::JNLE:
					return x86::Instruction::JLE;
					break;
				case x86::Instruction::JNO:
					return x86::Instruction::JO;
					break;
				case x86::Instruction::JNP:
					return x86::Instruction::JP;
					break;
				case x86::Instruction::JNS:
					return x86::Instruction::JS;
					break;
				case x86::Instruction::JNZ:
					return x86::Instruction::JZ;
					break;
				case x86::Instruction::JO:
					return x86::Instruction::JNO;
					break;
				case x86::Instruction::JP:
					return x86::Instruction::JNP;
					break;
				case x86::Instruction::JPE:
					return x86::Instruction::JNP;
					break;
				case x86::Instruction::JPO:
					return x86::Instruction::JPE;
					break;
				case x86::Instruction::JS:
					return x86::Instruction::JNS;
					break;
				case x86::Instruction::JZ:
					return x86::Instruction::JNZ;
					break;
				default:
					return x86::Instruction::UNKNOWN;
					break;
				}
			}
			void PeepHoleOptimize(Function_x86 & func)
			{
				for (auto iter = func.Code.begin(); iter != func.Code.end(); )
				{
					auto nextIter = std::next(iter);
					auto & instr = *iter;
					x86::Instruction * prevInstr = 0;
					x86::Instruction * nextInstr = 0;
					if (iter != func.Code.begin())
						prevInstr = &*std::prev(iter);
					if (nextIter != func.Code.end())
						nextInstr = &*nextIter;
					if (instr.Name == x86::Instruction::MOV &&
						instr.Op1 == instr.Op2)
					{
						iter = func.Code.erase(iter);
						continue;
					}
					if (instr.Name == x86::Instruction::MOV &&
						instr.Op1.Type == x86::OperandType::Register && instr.Op2.IsIntImmediate() && instr.Op2.Value == 0)
					{
						instr.Op2 = instr.Op1;
						instr.Name = x86::Instruction::XOR;
					}
					if (prevInstr && instr.Name == x86::Instruction::CMP && instr.Op2.IsIntImmediate() && instr.Op2.Value == 0
						&& instr.Op1 == prevInstr->Op1 && (prevInstr->Name == x86::Instruction::AND ||
						prevInstr->Name == x86::Instruction::OR || prevInstr->Name == x86::Instruction::ADD ||
						prevInstr->Name == x86::Instruction::SUB || prevInstr->Name == x86::Instruction::TEST ||
						prevInstr->Name == x86::Instruction::NOT) && nextInstr && 
						(nextInstr->Name == x86::Instruction::JE || nextInstr->Name == x86::Instruction::JNE ||
						nextInstr->Name == x86::Instruction::JZ || nextInstr->Name == x86::Instruction::JNZ))
					{
						iter = func.Code.erase(iter);
						continue;
					}
					if (prevInstr && nextInstr && prevInstr->IsJump() && prevInstr->Op1.Value == nextInstr->Label
						&& instr.Name == x86::Instruction::JMP)
					{
						auto oppo = GetOppositeJcc(prevInstr->Name);
						if (oppo != x86::Instruction::UNKNOWN)
						{
							prevInstr->Op1 = instr.Op1;
							prevInstr->Name = oppo;
							iter = func.Code.erase(iter);
						}
						else
						{
							iter = nextIter;
						}
						continue;
					}
					iter = nextIter;
				}
			}
		public:
			X86CodeGeneratorImpl()
			{
				labelGenerator = 0;
			}
			virtual Program_x86 GenerateCode(Program * program) override
			{
				Program_x86 rs;
				ptr_var_offset.clear();
				for (auto & func : program->Functions)
				{
					auto compiledFunc = GenerateFunction(func);
					PeepHoleOptimize(compiledFunc);
					rs.Functions.push_back(std::move(compiledFunc));
				}
				return rs;
			}
		};

		std::unique_ptr<X86CodeGenerator> CreateX86CodeGenerator()
		{
			return std::make_unique<X86CodeGeneratorImpl>();
		}
	}
}
