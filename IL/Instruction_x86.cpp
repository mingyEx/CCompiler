#include "CodeEmitter_x86.h"
namespace Compiler
{
	namespace x86
	{
		void BinaryCodeEmitter::Emit_ADC(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::Register && op1.Reg == Register::EAX) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\x15", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x80", 0, 2, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x83", 0, 2, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 8 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x10", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 32 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x11", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && op1.Type == OperandType::Register) && (op2.SizeBits == 8 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\x12", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\x13", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\x81", 0, 2, op1);
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_ADD(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::Register && op1.Reg == Register::EAX) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\x5", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x80", 0, 0, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x83", 0, 0, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 8 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x0", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 32 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x1", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && op1.Type == OperandType::Register) && (op2.SizeBits == 8 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\x2", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\x3", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\x81", 0, 0, op1);
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_AND(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::Register && op1.Reg == Register::EAX) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\x25", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x80", 0, 4, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x83", 0, 4, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 8 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x20", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 32 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x21", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && op1.Type == OperandType::Register) && (op2.SizeBits == 8 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\x22", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\x23", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\x81", 0, 4, op1);
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_BOUND(int paramCount, const Operand & op1, const Operand & op2)
		{
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_BSF(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\xbc", 0, op1.Reg, op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_BSR(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\xbd", 0, op1.Reg, op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_BSWAP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 32 && op1.Type == OperandType::Register))
			{
				EmitInstruction(2, "\xf\xc8", GetRegisterCode(op1.Reg), 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_BT(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 32 && op2.Type == OperandType::Register))
			{
				EmitInstruction(2, "\xf\xa3", 0, op2.Reg, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(2, "\xf\xba", 0, 4, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_BTC(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 32 && op2.Type == OperandType::Register))
			{
				EmitInstruction(2, "\xf\xbb", 0, op2.Reg, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(2, "\xf\xba", 0, 7, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_BTR(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 32 && op2.Type == OperandType::Register))
			{
				EmitInstruction(2, "\xf\xb3", 0, op2.Reg, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(2, "\xf\xba", 0, 6, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_BTS(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 32 && op2.Type == OperandType::Register))
			{
				EmitInstruction(2, "\xf\xab", 0, op2.Reg, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(2, "\xf\xba", 0, 5, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CALL(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\xe8", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			if (paramCount == 1 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xff", 0, 2, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CBW(int paramCount, const Operand & op1, const Operand & op2)
		{
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CDQ(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\x99", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CLC(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xf8", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CLD(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xfc", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CLI(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xfa", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMC(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xf5", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVA(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x47", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVAE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x43", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVB(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x42", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVBE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x46", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVC(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x42", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x44", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVG(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x4f", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVGE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x4d", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVL(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x4c", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVLE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x4e", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVNA(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x46", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVNAE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x42", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVNB(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x43", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVNBE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x47", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVNC(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x43", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVNE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x45", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVNG(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x4e", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVNGE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x4c", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVNL(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x4d", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVNLE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x4f", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVNO(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x41", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVNP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x4b", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVNS(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x49", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVNZ(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x45", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVO(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x40", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x4a", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVPE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x4a", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVPO(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x4b", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVS(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x48", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMOVZ(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x44", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::Register && op1.Reg == Register::EAX) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\x3d", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x80", 0, 7, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x83", 0, 7, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 8 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x38", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 32 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x39", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && op1.Type == OperandType::Register) && (op2.SizeBits == 8 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\x3a", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\x3b", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\x81", 0, 7, op1);
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMPSB(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xa6", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMPSD(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xa7", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CMPSW(int paramCount, const Operand & op1, const Operand & op2)
		{
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CWD(int paramCount, const Operand & op1, const Operand & op2)
		{
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_CWDE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\x98", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_DEC(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 32 && op1.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x48", GetRegisterCode(op1.Reg), 0, Operand());
				return;
			}
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xfe", 0, 1, op1);
				return;
			}
			if (paramCount == 1 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xff", 0, 1, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_DIV(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xf6", 0, 6, op1);
				return;
			}
			if (paramCount == 1 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xf7", 0, 6, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_F2XM1(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xf0", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FABS(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xe1", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FADD(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST && op1.Value == 0) && (op2.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xd8\xc0", op2.Value, 0, Operand());
				return;
			}
			if (paramCount == 2 && (op1.Type == OperandType::ST) && (op2.Type == OperandType::ST && op2.Value == 0))
			{
				EmitInstruction(2, "\xdc\xc0", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float32))
			{
				EmitInstruction(1, "\xd8", 0, 0, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float64))
			{
				EmitInstruction(1, "\xdc", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FADDP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST) && (op2.Type == OperandType::ST && op2.Value == 0))
			{
				EmitInstruction(2, "\xde\xc0", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xde\xc1", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FCHS(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xe0", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FCMOVB(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST && op1.Value == 0) && (op2.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xda\xc0", op2.Value, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FCMOVBE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST && op1.Value == 0) && (op2.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xda\xd0", op2.Value, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FCMOVE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST && op1.Value == 0) && (op2.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xda\xc8", op2.Value, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FCMOVNB(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST && op1.Value == 0) && (op2.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xdb\xc0", op2.Value, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FCMOVNBE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST && op1.Value == 0) && (op2.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xdb\xd0", op2.Value, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FCMOVNE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST && op1.Value == 0) && (op2.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xdb\xc8", op2.Value, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FCMOVNU(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST && op1.Value == 0) && (op2.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xdb\xd8", op2.Value, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FCMOVU(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST && op1.Value == 0) && (op2.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xda\xd8", op2.Value, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FCOM(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xd8\xd0", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd8\xd1", 0, 0, Operand());
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float32))
			{
				EmitInstruction(1, "\xd8", 0, 2, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float64))
			{
				EmitInstruction(1, "\xdc", 0, 2, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FCOMI(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST && op1.Value == 0) && (op2.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xdb\xf0", op2.Value, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FCOMIP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST && op1.Value == 0) && (op2.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xdf\xf0", op2.Value, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FCOMP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xd8\xd8", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd8\xd9", 0, 0, Operand());
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float32))
			{
				EmitInstruction(1, "\xd8", 0, 3, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float64))
			{
				EmitInstruction(1, "\xdc", 0, 3, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FCOMPP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xde\xd9", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FCOS(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xff", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FDECSTP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xf6", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FDIV(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST && op1.Value == 0) && (op2.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xd8\xf0", op2.Value, 0, Operand());
				return;
			}
			if (paramCount == 2 && (op1.Type == OperandType::ST) && (op2.Type == OperandType::ST && op2.Value == 0))
			{
				EmitInstruction(2, "\xdc\xf8", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float32))
			{
				EmitInstruction(1, "\xd8", 0, 6, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float64))
			{
				EmitInstruction(1, "\xdc", 0, 6, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FDIVP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST) && (op2.Type == OperandType::ST && op2.Value == 0))
			{
				EmitInstruction(2, "\xde\xf8", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xde\xf9", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FDIVR(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST && op1.Value == 0) && (op2.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xd8\xf8", op2.Value, 0, Operand());
				return;
			}
			if (paramCount == 2 && (op1.Type == OperandType::ST) && (op2.Type == OperandType::ST && op2.Value == 0))
			{
				EmitInstruction(2, "\xdc\xf0", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float32))
			{
				EmitInstruction(1, "\xd8", 0, 7, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float64))
			{
				EmitInstruction(1, "\xdc", 0, 7, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FDIVRP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST) && (op2.Type == OperandType::ST && op2.Value == 0))
			{
				EmitInstruction(2, "\xde\xf0", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xde\xf1", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FIADD(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int32))
			{
				EmitInstruction(1, "\xda", 0, 0, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int16))
			{
				EmitInstruction(1, "\xde", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FICOM(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int16))
			{
				EmitInstruction(1, "\xde", 0, 2, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int32))
			{
				EmitInstruction(1, "\xda", 0, 2, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FICOMP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int16))
			{
				EmitInstruction(1, "\xde", 0, 3, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int32))
			{
				EmitInstruction(1, "\xda", 0, 3, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FIDIV(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int32))
			{
				EmitInstruction(1, "\xda", 0, 6, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int16))
			{
				EmitInstruction(1, "\xde", 0, 6, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FIDIVR(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int32))
			{
				EmitInstruction(1, "\xda", 0, 7, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int16))
			{
				EmitInstruction(1, "\xde", 0, 7, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FILD(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int16))
			{
				EmitInstruction(1, "\xdf", 0, 0, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int32))
			{
				EmitInstruction(1, "\xdb", 0, 0, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int64))
			{
				EmitInstruction(1, "\xdf", 0, 5, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FIMUL(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int32))
			{
				EmitInstruction(1, "\xda", 0, 1, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int16))
			{
				EmitInstruction(1, "\xde", 0, 1, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FINCSTP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xf7", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FINIT(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(3, "\x9b\xdb\xe3", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FIST(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int16))
			{
				EmitInstruction(1, "\xdf", 0, 2, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int32))
			{
				EmitInstruction(1, "\xdb", 0, 2, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FISTP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int16))
			{
				EmitInstruction(1, "\xdf", 0, 3, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int32))
			{
				EmitInstruction(1, "\xdb", 0, 3, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int64))
			{
				EmitInstruction(1, "\xdf", 0, 7, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FISTTP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int16))
			{
				EmitInstruction(1, "\xdf", 0, 1, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int32))
			{
				EmitInstruction(1, "\xdb", 0, 1, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int64))
			{
				EmitInstruction(1, "\xdd", 0, 1, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FISUB(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int32))
			{
				EmitInstruction(1, "\xda", 0, 4, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int16))
			{
				EmitInstruction(1, "\xde", 0, 4, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FISUBR(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int32))
			{
				EmitInstruction(1, "\xda", 0, 5, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int16))
			{
				EmitInstruction(1, "\xde", 0, 5, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FLD(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xd9\xc0", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float32))
			{
				EmitInstruction(1, "\xd9", 0, 0, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float64))
			{
				EmitInstruction(1, "\xdd", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FLD1(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xe8", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FLDL2E(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xea", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FLDL2T(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xe9", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FLDLG2(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xec", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FLDLN2(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xed", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FLDPI(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xeb", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FLDZ(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xee", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FMUL(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST && op1.Value == 0) && (op2.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xd8\xc8", op2.Value, 0, Operand());
				return;
			}
			if (paramCount == 2 && (op1.Type == OperandType::ST) && (op2.Type == OperandType::ST && op2.Value == 0))
			{
				EmitInstruction(2, "\xdc\xc8", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float32))
			{
				EmitInstruction(1, "\xd8", 0, 1, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float64))
			{
				EmitInstruction(1, "\xdc", 0, 1, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FMULP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST) && (op2.Type == OperandType::ST && op2.Value == 0))
			{
				EmitInstruction(2, "\xde\xc8", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xde\xc9", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FNINIT(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xdb\xe3", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FNSTSW(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int16))
			{
				EmitInstruction(1, "\xdd", 0, 7, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FPATAN(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xf3", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FRNDINT(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xfc", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FSCALE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xfd", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FSIN(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xfe", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FSINCOS(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xfb", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FSQRT(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xfa", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FST(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xdd\xd0", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float32))
			{
				EmitInstruction(1, "\xd9", 0, 2, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float64))
			{
				EmitInstruction(1, "\xdd", 0, 2, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FSTP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xdd\xd8", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float32))
			{
				EmitInstruction(1, "\xd9", 0, 3, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float64))
			{
				EmitInstruction(1, "\xdd", 0, 3, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FSTSW(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Int16))
			{
				EmitInstruction(2, "\x9b\xdd", 0, 7, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FSUB(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST && op1.Value == 0) && (op2.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xd8\xe0", op2.Value, 0, Operand());
				return;
			}
			if (paramCount == 2 && (op1.Type == OperandType::ST) && (op2.Type == OperandType::ST && op2.Value == 0))
			{
				EmitInstruction(2, "\xdc\xe8", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float32))
			{
				EmitInstruction(1, "\xd8", 0, 4, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float64))
			{
				EmitInstruction(1, "\xdc", 0, 4, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FSUBP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST) && (op2.Type == OperandType::ST && op2.Value == 0))
			{
				EmitInstruction(2, "\xde\xe8", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xde\xe9", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FSUBR(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST && op1.Value == 0) && (op2.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xd8\xe8", op2.Value, 0, Operand());
				return;
			}
			if (paramCount == 2 && (op1.Type == OperandType::ST) && (op2.Type == OperandType::ST && op2.Value == 0))
			{
				EmitInstruction(2, "\xdc\xe0", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float32))
			{
				EmitInstruction(1, "\xd8", 0, 5, op1);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Memory && op1.MemoryType == MemoryOperandType::Float64))
			{
				EmitInstruction(1, "\xdc", 0, 5, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FSUBRP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST) && (op2.Type == OperandType::ST && op2.Value == 0))
			{
				EmitInstruction(2, "\xde\xe0", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xde\xe1", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FTST(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xe4", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FUCOM(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xdd\xe0", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xdd\xe1", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FUCOMI(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST && op1.Value == 0) && (op2.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xdb\xe8", op2.Value, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FUCOMIP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::ST && op1.Value == 0) && (op2.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xdf\xe8", op2.Value, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FUCOMP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xdd\xe8", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xdd\xe9", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FUCOMPP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xda\xe9", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FXAM(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xe5", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FXCH(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::ST))
			{
				EmitInstruction(2, "\xd9\xc8", op1.Value, 0, Operand());
				return;
			}
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xc9", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FXTRACT(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xf4", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FYL2X(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xf1", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_FYL2XP1(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(2, "\xd9\xf9", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_IDIV(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xf6", 0, 7, op1);
				return;
			}
			if (paramCount == 1 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xf7", 0, 7, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_IMUL(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xf6", 0, 5, op1);
				return;
			}
			if (paramCount == 1 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xf7", 0, 5, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\xaf", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && op2.Type == OperandType::Immediate32)
			{
				EmitInstruction(2, "\x66\x69", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_INC(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 32 && op1.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x40", GetRegisterCode(op1.Reg), 0, Operand());
				return;
			}
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xfe", 0, 0, op1);
				return;
			}
			if (paramCount == 1 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xff", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_INT(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && ((op1.Type == OperandType::Immediate32 || op1.Type == OperandType::Immediate8) && op1.Value == 3))
			{
				EmitInstruction(1, "\xcc", 0, 0, Operand());
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xcd", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_INTO(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xce", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JA(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x77", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x87", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JAE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x73", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x83", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JB(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x72", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x82", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JBE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x76", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x86", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JC(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x72", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x82", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JCXZ(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xe3", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x74", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x84", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JECXZ(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xe3", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JG(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x7f", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x8f", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JGE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x7d", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x8d", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JL(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x7c", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x8c", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JLE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x7e", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x8e", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JMP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xeb", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\xe9", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			if (paramCount == 1 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xff", 0, 4, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JNA(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x76", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x86", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JNAE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x72", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x82", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JNB(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x73", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x83", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JNBE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x77", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x87", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JNC(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x73", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x83", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JNE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x75", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x85", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JNG(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x7e", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x8e", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JNGE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x7c", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x8c", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JNL(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x7d", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x8d", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JNLE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x7f", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x8f", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JNO(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x71", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x81", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JNP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x7b", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x8b", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JNS(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x79", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x89", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JNZ(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x75", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x85", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JO(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x70", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x80", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x7a", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x8a", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JPE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x7a", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x8a", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JPO(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x7b", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x8b", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JS(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x78", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x88", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_JZ(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x74", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x84", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(2, "\xf\x84", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_LEA(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.Type == OperandType::Memory && op2.MemoryType == MemoryOperandType::Int32))
			{
				EmitInstruction(1, "\x8d", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_LODSB(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xac", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_LODSD(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xad", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_LODSW(int paramCount, const Operand & op1, const Operand & op2)
		{
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_LOOP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xe2", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_LOOPE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xe1", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_LOOPNE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xe0", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_LOOPNZ(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xe0", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_LOOPZ(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xe1", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_MOV(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 8 && op1.Type == OperandType::Register) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xb0", GetRegisterCode(op1.Reg), 0, Operand());
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\xb8", GetRegisterCode(op1.Reg), 0, Operand());
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 8 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x88", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 32 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x89", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && op1.Type == OperandType::Register) && (op2.SizeBits == 8 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\x8a", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\x8b", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xc6", 0, 0, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\xc7", 0, 0, op1);
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_MOVSB(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xa4", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_MOVSD(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xa5", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_MOVSW(int paramCount, const Operand & op1, const Operand & op2)
		{
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_MOVZX(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 8 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\xb6", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 16 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\xb7", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_MUL(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xf6", 0, 4, op1);
				return;
			}
			if (paramCount == 1 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xf7", 0, 4, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_NEG(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xf6", 0, 3, op1);
				return;
			}
			if (paramCount == 1 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xf7", 0, 3, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_NOT(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xf6", 0, 2, op1);
				return;
			}
			if (paramCount == 1 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xf7", 0, 2, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_OR(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::Register && op1.Reg == Register::EAX) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\xd", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x80", 0, 1, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x83", 0, 1, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 8 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x8", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 32 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x9", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && op1.Type == OperandType::Register) && (op2.SizeBits == 8 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xa", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xb", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\x81", 0, 1, op1);
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_POP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 32 && op1.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x58", GetRegisterCode(op1.Reg), 0, Operand());
				return;
			}
			if (paramCount == 1 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\x8f", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_POPA(int paramCount, const Operand & op1, const Operand & op2)
		{
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_POPAD(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\x61", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_PUSH(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 32 && op1.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x50", GetRegisterCode(op1.Reg), 0, Operand());
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x6a", 0, 0, Operand());
				code.push_back((unsigned char)op1.Value);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\x68", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				code.push_back(((unsigned char*)&(op1.Value))[2]);
				code.push_back(((unsigned char*)&(op1.Value))[3]);
				return;
			}
			if (paramCount == 1 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xff", 0, 6, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_PUSHA(int paramCount, const Operand & op1, const Operand & op2)
		{
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_PUSHAD(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\x60", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_RCL(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && ((op2.Type == OperandType::Immediate32 || op2.Type == OperandType::Immediate8) && op2.Value == 1))
			{
				EmitInstruction(1, "\xd0", 0, 2, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Register && op2.Reg == Register::ECX))
			{
				EmitInstruction(1, "\xd2", 0, 2, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && ((op2.Type == OperandType::Immediate32 || op2.Type == OperandType::Immediate8) && op2.Value == 1))
			{
				EmitInstruction(1, "\xd1", 0, 2, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Register && op2.Reg == Register::ECX))
			{
				EmitInstruction(1, "\xd3", 0, 2, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xc0", 0, 2, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xc1", 0, 2, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_RCR(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && ((op2.Type == OperandType::Immediate32 || op2.Type == OperandType::Immediate8) && op2.Value == 1))
			{
				EmitInstruction(1, "\xd0", 0, 3, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Register && op2.Reg == Register::ECX))
			{
				EmitInstruction(1, "\xd2", 0, 3, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && ((op2.Type == OperandType::Immediate32 || op2.Type == OperandType::Immediate8) && op2.Value == 1))
			{
				EmitInstruction(1, "\xd1", 0, 3, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Register && op2.Reg == Register::ECX))
			{
				EmitInstruction(1, "\xd3", 0, 3, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xc0", 0, 3, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xc1", 0, 3, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_RET(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xc3", 0, 0, Operand());
				return;
			}
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xcb", 0, 0, Operand());
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate16))
			{
				EmitInstruction(1, "\xc2", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				return;
			}
			if (paramCount == 1 && (op1.Type == OperandType::Immediate16))
			{
				EmitInstruction(1, "\xca", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op1.Value))[0]);
				code.push_back(((unsigned char*)&(op1.Value))[1]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_ROL(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && ((op2.Type == OperandType::Immediate32 || op2.Type == OperandType::Immediate8) && op2.Value == 1))
			{
				EmitInstruction(1, "\xd0", 0, 0, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Register && op2.Reg == Register::ECX))
			{
				EmitInstruction(1, "\xd2", 0, 0, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && ((op2.Type == OperandType::Immediate32 || op2.Type == OperandType::Immediate8) && op2.Value == 1))
			{
				EmitInstruction(1, "\xd1", 0, 0, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Register && op2.Reg == Register::ECX))
			{
				EmitInstruction(1, "\xd3", 0, 0, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xc0", 0, 0, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xc1", 0, 0, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_ROR(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && ((op2.Type == OperandType::Immediate32 || op2.Type == OperandType::Immediate8) && op2.Value == 1))
			{
				EmitInstruction(1, "\xd0", 0, 1, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Register && op2.Reg == Register::ECX))
			{
				EmitInstruction(1, "\xd2", 0, 1, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && ((op2.Type == OperandType::Immediate32 || op2.Type == OperandType::Immediate8) && op2.Value == 1))
			{
				EmitInstruction(1, "\xd1", 0, 1, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Register && op2.Reg == Register::ECX))
			{
				EmitInstruction(1, "\xd3", 0, 1, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xc0", 0, 1, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xc1", 0, 1, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SAHF(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\x9e", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SAL(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xd0", 0, 4, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Register && op2.Reg == Register::ECX))
			{
				EmitInstruction(1, "\xd2", 0, 4, op1);
				return;
			}
			if (paramCount == 1 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xd1", 0, 4, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Register && op2.Reg == Register::ECX))
			{
				EmitInstruction(1, "\xd3", 0, 4, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xc0", 0, 4, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xc1", 0, 4, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SAR(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xd0", 0, 7, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Register && op2.Reg == Register::ECX))
			{
				EmitInstruction(1, "\xd2", 0, 7, op1);
				return;
			}
			if (paramCount == 1 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xd1", 0, 7, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Register && op2.Reg == Register::ECX))
			{
				EmitInstruction(1, "\xd3", 0, 7, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xc0", 0, 7, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xc1", 0, 7, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SBB(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::Register && op1.Reg == Register::EAX) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\x1d", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x80", 0, 3, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x83", 0, 3, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 8 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x18", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 32 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x19", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && op1.Type == OperandType::Register) && (op2.SizeBits == 8 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\x1a", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\x1b", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\x81", 0, 3, op1);
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SCASB(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xae", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SCASD(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xaf", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SCASW(int paramCount, const Operand & op1, const Operand & op2)
		{
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETA(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x97", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETAE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x93", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETB(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x92", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETBE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x96", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETC(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x92", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x94", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETG(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x9f", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETGE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x9d", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETL(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x9c", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETLE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x9e", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETNA(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x96", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETNAE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x92", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETNB(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x93", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETNBE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x97", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETNC(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x93", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETNE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x95", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETNG(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x9e", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETNGE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x9c", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETNL(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x9d", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETNLE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x9f", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETNO(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x91", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETNP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x9b", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETNS(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x99", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETNZ(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x95", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETO(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x90", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETP(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x9a", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETPE(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x9a", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETPO(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x9b", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETS(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x98", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SETZ(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(2, "\xf\x94", 0, 0, op1);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SHL(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xd0", 0, 4, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Register && op2.Reg == Register::ECX))
			{
				EmitInstruction(1, "\xd2", 0, 4, op1);
				return;
			}
			if (paramCount == 1 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xd1", 0, 4, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Register && op2.Reg == Register::ECX))
			{
				EmitInstruction(1, "\xd3", 0, 4, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && op2.Type == OperandType::Immediate32 && op2.Value == 1)
			{
				EmitInstruction(1, "\xd1", 0, 4, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xc0", 0, 4, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\xc1", 0, 4, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SHR(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 1 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xd0", 0, 5, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Register && op2.Reg == Register::ECX))
			{
				EmitInstruction(1, "\xd2", 0, 5, op1);
				return;
			}
			if (paramCount == 1 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\xd1", 0, 5, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Register && op2.Reg == Register::ECX))
			{
				EmitInstruction(1, "\xd3", 0, 5, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && op2.Type == OperandType::Immediate32 && op2.Value == 1)
			{
				EmitInstruction(1, "\xd1", 0, 5, op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xc0", 0, 5, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\xc1", 0, 5, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_STC(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xf9", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_STD(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xfd", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_STI(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xfb", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_STOSB(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xaa", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_STOSD(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 0)
			{
				EmitInstruction(1, "\xab", 0, 0, Operand());
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_STOSW(int paramCount, const Operand & op1, const Operand & op2)
		{
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_SUB(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::Register && op1.Reg == Register::EAX) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\x2d", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x80", 0, 5, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x83", 0, 5, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 8 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x28", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 32 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x29", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && op1.Type == OperandType::Register) && (op2.SizeBits == 8 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\x2a", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\x2b", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\x81", 0, 5, op1);
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_TEST(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::Register && op1.Reg == Register::EAX) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\xa9", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\xf6", 0, 0, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 8 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x84", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 32 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x85", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\xf7", 0, 0, op1);
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_XCHG(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::Register && op1.Reg == Register::EAX) && (op2.SizeBits == 32 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x90", GetRegisterCode(op2.Reg), 0, Operand());
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.Type == OperandType::Register && op2.Reg == Register::EAX))
			{
				EmitInstruction(1, "\x90", GetRegisterCode(op1.Reg), 0, Operand());
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 8 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x86", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && op1.Type == OperandType::Register) && (op2.SizeBits == 8 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\x86", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 32 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x87", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\x87", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

		void BinaryCodeEmitter::Emit_XOR(int paramCount, const Operand & op1, const Operand & op2)
		{
			if (paramCount == 2 && (op1.Type == OperandType::Register && op1.Reg == Register::EAX) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\x35", 0, 0, Operand());
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x80", 0, 6, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate8))
			{
				EmitInstruction(1, "\x83", 0, 6, op1);
				code.push_back((unsigned char)op2.Value);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 8 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x30", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.SizeBits == 32 && op2.Type == OperandType::Register))
			{
				EmitInstruction(1, "\x31", 0, GetRegisterCode(op2.Reg), op1);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 8 && op1.Type == OperandType::Register) && (op2.SizeBits == 8 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\x32", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && op1.Type == OperandType::Register) && (op2.SizeBits == 32 && (op2.Type == OperandType::Register || op2.Type == OperandType::Memory)))
			{
				EmitInstruction(1, "\x33", 0, GetRegisterCode(op1.Reg), op2);
				return;
			}
			if (paramCount == 2 && (op1.SizeBits == 32 && (op1.Type == OperandType::Register || op1.Type == OperandType::Memory)) && (op2.Type == OperandType::Immediate32))
			{
				EmitInstruction(1, "\x81", 0, 6, op1);
				code.push_back(((unsigned char*)&(op2.Value))[0]);
				code.push_back(((unsigned char*)&(op2.Value))[1]);
				code.push_back(((unsigned char*)&(op2.Value))[2]);
				code.push_back(((unsigned char*)&(op2.Value))[3]);
				return;
			}
			throw InvalidProgramException(L"Illegal operands.");
		}

	}
}
