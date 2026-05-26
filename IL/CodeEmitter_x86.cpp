
#include "CodeEmitter_x86.h"
#include <cstring>
#include <stdio.h>

namespace Compiler
{
	namespace x86
	{
		int GetMemoryTypeSize(MemoryOperandType memType)
		{
			switch (memType)
			{
			case MemoryOperandType::Int8:
				return 8;
			case MemoryOperandType::Int16:
				return 16;
			case MemoryOperandType::Int32:
				return 32;
			case MemoryOperandType::Int64:
				return 64;
			case MemoryOperandType::Float32:
				return 32;
			case MemoryOperandType::Float64:
				return 64;
			default:
				return 0;
			}
		}

		Operand Operand::FromMemory(MemoryOperandType memType, int displacement)
		{
			Operand rs;
			rs.Type = OperandType::Memory;
			rs.AddressingMode = OperandAddressingMode::Disp32;
			rs.Value = displacement;
			rs.MemoryType = memType;
			rs.SizeBits = GetMemoryTypeSize(memType);
			return rs;
		}

		// [baseReg + displacement]
		Operand Operand::FromMemory(MemoryOperandType memType, Register reg, int displacement)
		{
			if (reg == Register::None)
				throw InvalidProgramException(L"Illegal register");
			if (reg == Register::ESP)
				return FromMemory(memType, reg, Register::None, 1, displacement);
			Operand rs;
			rs.Type = OperandType::Memory;
			rs.Reg = reg;
			rs.Value = displacement;
			rs.MemoryType = memType;
			rs.SizeBits = GetMemoryTypeSize(memType);
			if (displacement == 0 && reg != Register::ESP && reg != Register::EBP)
			{
				rs.AddressingMode = OperandAddressingMode::RegisterPointer;
				return rs;
			}
			if (displacement >= CHAR_MIN && displacement <= CHAR_MAX)
			{
				rs.AddressingMode = OperandAddressingMode::RegisterDisp8;
			}
			else
			{
				rs.AddressingMode = OperandAddressingMode::RegisterDisp32;
			}
			return rs;
		}

		// [baseReg + indexReg*scale + displacement]
		Operand Operand::FromMemory(MemoryOperandType memType, Register baseReg, Register indexReg, int scale, int displacement)
		{
			if (baseReg == Register::None)
				throw InvalidProgramException(L"Illegal register");
			if (indexReg == Register::ESP)
				throw InvalidProgramException(L"ESP cannot be index.");
			if (scale != 1 && scale != 4 && scale != 8 && scale != 2)
				throw InvalidProgramException(L"Scale must be 1, 2, 4 and 8.");
			Operand rs;
			rs.Type = OperandType::Memory;
			rs.Reg = baseReg;
			rs.IndexReg = indexReg;
			rs.Scale = scale;
			rs.Value = displacement;
			rs.MemoryType = memType;
			rs.SizeBits = GetMemoryTypeSize(memType);
			if (displacement == 0 && baseReg != Register::EBP)
			{
				rs.AddressingMode = OperandAddressingMode::SIB;
				return rs;
			}
			if (displacement >= CHAR_MIN && displacement <= CHAR_MAX)
			{
				rs.AddressingMode = OperandAddressingMode::SIBDisp8;
			}
			else
			{
				rs.AddressingMode = OperandAddressingMode::SIBDisp32;
			}
			return rs;
		}

		unsigned char Operand::GetSIB()
		{
			unsigned char base, ss, index;
			if (Scale == 1)
				ss = 0;
			else if (Scale == 2)
				ss = 1;
			else if (Scale == 4)
				ss = 2;
			else if (Scale == 8)
				ss = 3;
			else
			{
				throw InvalidProgramException(L"Scale can only be 1, 2, 4, and 8.");
			}
			if (IndexReg == Register::None)
				index = 4;
			else if (IndexReg != Register::ESP)
				index = GetRegisterCode(IndexReg);
			else
			{
				throw InvalidProgramException(L"Index register cannot be ESP.");
			}
			if (Reg == Register::None)
				throw InvalidProgramException(L"Invalid register for base.");
			else if (Reg == Register::EBP && AddressingMode == OperandAddressingMode::SIB)
				throw InvalidProgramException(L"EBP cannot be used as base in pure SIB mode.");
			base = GetRegisterCode(Reg);
			return (ss<<6)+(index<<3)+base;
		}

		void Operand::GetOpCode(unsigned char &rm, unsigned char & mod, bool & hasSIB, unsigned char & sib, int & postSize, unsigned char postBytes[4])
		{
			hasSIB = false;
			postSize = 0;
			rm = 0;
			mod = 0;
			sib = 0;
			if (Type == OperandType::None)
				return;
			if (Type == OperandType::Immediate16)
			{
				postSize = 2;
				unsigned char * bytes = (unsigned char*)&Value;
				postBytes[0] = bytes[0];
				postBytes[1] = bytes[1];
			}
			else if (Type == OperandType::Immediate32)
			{
				postSize = 4;
				unsigned char * bytes = (unsigned char*)&Value;
				postBytes[0] = bytes[0];
				postBytes[1] = bytes[1];
				postBytes[2] = bytes[2];
				postBytes[3] = bytes[3];
			}
			else if (Type == OperandType::Immediate8)
			{
				postSize = 1;
				unsigned char * bytes = (unsigned char*)&Value;
				postBytes[0] = bytes[0];
			}
			else if (Type == OperandType::Register)
			{
				mod = 3;
				rm = GetRegisterCode(Reg);
			}
			else if (Type == OperandType::Memory)
			{
				switch (AddressingMode)
				{
				case OperandAddressingMode::RegisterPointer:
					mod = 0;
					if (Reg == Register::ESP)
						throw InvalidProgramException(L"[ESP] addressing mode is invalid.");
					if (Reg == Register::EBP)
						throw InvalidProgramException(L"[EBP] addressing mode is invalid.");
					rm = GetRegisterCode(Reg);
					break;
				case OperandAddressingMode::SIB:
					mod = 0;
					rm = 4;
					sib = GetSIB();
					hasSIB = true;
					break;
				case OperandAddressingMode::Disp32:
					mod = 0;
					rm = 5;
					postSize = 4;
					postBytes[0] = ((unsigned char*)&Value)[0];
					postBytes[1] = ((unsigned char*)&Value)[1];
					postBytes[2] = ((unsigned char*)&Value)[2];
					postBytes[3] = ((unsigned char*)&Value)[3];
					break;
				case OperandAddressingMode::RegisterDisp8:
					mod = 1;
					rm = GetRegisterCode(Reg);
					postSize = 1;
					postBytes[0] = ((unsigned char*)&Value)[0];
					if (Reg == Register::ESP)
						throw InvalidProgramException(L"disp8[ESP] addressing mode is invalid.");
					break;
				case OperandAddressingMode::SIBDisp8:
					mod = 1;
					rm = 4;
					sib = GetSIB();
					hasSIB = true;
					postSize = 1;
					postBytes[0] = ((unsigned char*)&Value)[0];
					break;
				case OperandAddressingMode::RegisterDisp32:
					mod = 2;
					if (Reg == Register::ESP)
						throw InvalidProgramException(L"disp32[ESP] addressing mode is invalid.");
					rm = GetRegisterCode(Reg);
					postSize = 4;
					postBytes[0] = ((unsigned char*)&Value)[0];
					postBytes[1] = ((unsigned char*)&Value)[1];
					postBytes[2] = ((unsigned char*)&Value)[2];
					postBytes[3] = ((unsigned char*)&Value)[3];
					break;
				case OperandAddressingMode::SIBDisp32:
					mod = 2;
					rm = 4;
					sib = GetSIB();
					hasSIB = true;
					postSize = 4;
					postBytes[0] = ((unsigned char*)&Value)[0];
					postBytes[1] = ((unsigned char*)&Value)[1];
					postBytes[2] = ((unsigned char*)&Value)[2];
					postBytes[3] = ((unsigned char*)&Value)[3];
					break;
				default:
					break;
				}
			}
		}

		void BinaryCodeEmitter::EmitInstruction(int opLen, const char * opCode, unsigned char opCodePlus, unsigned char extDigit, Operand op)
		{
			for (int i = 0; i<opLen; i++)
				if (i==opLen-1)
					code.push_back(opCode[i] + opCodePlus);
				else
					code.push_back(opCode[i]);

			unsigned char rmByte, sib;
			unsigned char mod, rm;
			bool hasSIB = false;
			int postSize = 0;
			unsigned char postBytes[4];
			if (op.Type != OperandType::None)
			{
				op.GetOpCode(rm, mod, hasSIB, sib, postSize, postBytes);
				rmByte = (mod<<6) + (extDigit<<3) + rm;
				code.push_back(rmByte);
			}
			if (hasSIB)
				code.push_back(sib);
			for (int i = 0; i<postSize; i++)
				code.push_back(postBytes[i]);
		}

		void BinaryCodeEmitter::EmitInstruction(int opLen, const char * opCode, unsigned char opCodePlus, Register reg, Operand op)
		{
			EmitInstruction(opLen, opCode, opCodePlus, GetRegisterCode(reg), op);
		}

		void BinaryCodeEmitter::EmitLoad(Register dest, int offset) // dest = [EBP+offset]
		{
			if (offset>=CHAR_MIN && offset <= CHAR_MAX)
			{
				Emit_MOV(2, Operand(dest), Operand::FromMemory(MemoryOperandType::Int32, Register::EBP, offset));
			}
			else
			{
				Emit_MOV(2, Operand(dest), Operand::FromMemory(MemoryOperandType::Int32, Register::EBP, offset));
			}
		}
		void BinaryCodeEmitter::EmitStore(Register source, int offset) // [EBP+offset] = source
		{
			if (offset>=CHAR_MIN && offset <= CHAR_MAX)
			{
				Emit_MOV(2, Operand::FromMemory(MemoryOperandType::Int32, Register::EBP, offset), Operand(source));
			}
			else
			{
				Emit_MOV(2, Operand::FromMemory(MemoryOperandType::Int32, Register::EBP, offset), Operand(source));
			}
		}

		void * BinaryCodeEmitter::ToFunc()
		{
			void * addr;
			addr = VirtualAlloc(0, code.size(), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			std::memcpy(addr, code.data(), code.size());
			VirtualProtect(addr, code.size(), PAGE_EXECUTE, 0);
			return addr;
		}

		void BinaryCodeEmitter::Emit(const Instruction & instr)
		{
			switch (instr.Name)
			{
			case Instruction::ADC:
				Emit_ADC(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::ADD:
				Emit_ADD(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::AND:
				Emit_AND(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::BOUND:
				Emit_BOUND(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::BSF:
				Emit_BSF(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::BSR:
				Emit_BSR(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::BSWAP:
				Emit_BSWAP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::BT:
				Emit_BT(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::BTC:
				Emit_BTC(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::BTR:
				Emit_BTR(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::BTS:
				Emit_BTS(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CALL:
				Emit_CALL(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CBW:
				Emit_CBW(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CDQ:
				Emit_CDQ(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CLC:
				Emit_CLC(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CLD:
				Emit_CLD(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CLI:
				Emit_CLI(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMC:
				Emit_CMC(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVA:
				Emit_CMOVA(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVAE:
				Emit_CMOVAE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVB:
				Emit_CMOVB(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVBE:
				Emit_CMOVBE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVC:
				Emit_CMOVC(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVE:
				Emit_CMOVE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVG:
				Emit_CMOVG(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVGE:
				Emit_CMOVGE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVL:
				Emit_CMOVL(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVLE:
				Emit_CMOVLE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVNA:
				Emit_CMOVNA(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVNAE:
				Emit_CMOVNAE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVNB:
				Emit_CMOVNB(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVNBE:
				Emit_CMOVNBE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVNC:
				Emit_CMOVNC(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVNE:
				Emit_CMOVNE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVNG:
				Emit_CMOVNG(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVNGE:
				Emit_CMOVNGE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVNL:
				Emit_CMOVNL(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVNLE:
				Emit_CMOVNLE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVNO:
				Emit_CMOVNO(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVNP:
				Emit_CMOVNP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVNS:
				Emit_CMOVNS(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVNZ:
				Emit_CMOVNZ(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVO:
				Emit_CMOVO(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVP:
				Emit_CMOVP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVPE:
				Emit_CMOVPE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVPO:
				Emit_CMOVPO(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVS:
				Emit_CMOVS(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMOVZ:
				Emit_CMOVZ(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMP:
				Emit_CMP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMPSB:
				Emit_CMPSB(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMPSD:
				Emit_CMPSD(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CMPSW:
				Emit_CMPSW(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CWD:
				Emit_CWD(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::CWDE:
				Emit_CWDE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::DEC:
				Emit_DEC(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::DIV:
				Emit_DIV(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::F2XM1:
				Emit_F2XM1(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FABS:
				Emit_FABS(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FADD:
				Emit_FADD(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FADDP:
				Emit_FADDP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FCHS:
				Emit_FCHS(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FCMOVB:
				Emit_FCMOVB(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FCMOVBE:
				Emit_FCMOVBE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FCMOVE:
				Emit_FCMOVE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FCMOVNB:
				Emit_FCMOVNB(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FCMOVNBE:
				Emit_FCMOVNBE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FCMOVNE:
				Emit_FCMOVNE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FCMOVNU:
				Emit_FCMOVNU(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FCMOVU:
				Emit_FCMOVU(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FCOM:
				Emit_FCOM(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FCOMI:
				Emit_FCOMI(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FCOMIP:
				Emit_FCOMIP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FCOMP:
				Emit_FCOMP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FCOMPP:
				Emit_FCOMPP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FCOS:
				Emit_FCOS(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FDECSTP:
				Emit_FDECSTP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FDIV:
				Emit_FDIV(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FDIVP:
				Emit_FDIVP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FDIVR:
				Emit_FDIVR(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FDIVRP:
				Emit_FDIVRP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FIADD:
				Emit_FIADD(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FICOM:
				Emit_FICOM(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FICOMP:
				Emit_FICOMP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FIDIV:
				Emit_FIDIV(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FIDIVR:
				Emit_FIDIVR(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FILD:
				Emit_FILD(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FIMUL:
				Emit_FIMUL(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FINCSTP:
				Emit_FINCSTP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FINIT:
				Emit_FINIT(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FIST:
				Emit_FIST(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FISTP:
				Emit_FISTP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FISTTP:
				Emit_FISTTP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FISUB:
				Emit_FISUB(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FISUBR:
				Emit_FISUBR(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FLD:
				Emit_FLD(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FLD1:
				Emit_FLD1(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FLDL2E:
				Emit_FLDL2E(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FLDL2T:
				Emit_FLDL2T(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FLDLG2:
				Emit_FLDLG2(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FLDLN2:
				Emit_FLDLN2(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FLDPI:
				Emit_FLDPI(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FLDZ:
				Emit_FLDZ(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FMUL:
				Emit_FMUL(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FMULP:
				Emit_FMULP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FNINIT:
				Emit_FNINIT(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FNSTSW:
				Emit_FNSTSW(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FPATAN:
				Emit_FPATAN(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FRNDINT:
				Emit_FRNDINT(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FSCALE:
				Emit_FSCALE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FSIN:
				Emit_FSIN(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FSINCOS:
				Emit_FSINCOS(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FSQRT:
				Emit_FSQRT(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FST:
				Emit_FST(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FSTP:
				Emit_FSTP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FSTSW:
				Emit_FSTSW(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FSUB:
				Emit_FSUB(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FSUBP:
				Emit_FSUBP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FSUBR:
				Emit_FSUBR(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FSUBRP:
				Emit_FSUBRP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FTST:
				Emit_FTST(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FUCOM:
				Emit_FUCOM(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FUCOMI:
				Emit_FUCOMI(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FUCOMIP:
				Emit_FUCOMIP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FUCOMP:
				Emit_FUCOMP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FUCOMPP:
				Emit_FUCOMPP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FXAM:
				Emit_FXAM(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FXCH:
				Emit_FXCH(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FXTRACT:
				Emit_FXTRACT(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FYL2X:
				Emit_FYL2X(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::FYL2XP1:
				Emit_FYL2XP1(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::IDIV:
				Emit_IDIV(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::IMUL:
				Emit_IMUL(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::INC:
				Emit_INC(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::INT:
				Emit_INT(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::INTO:
				Emit_INTO(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JA:
				Emit_JA(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JAE:
				Emit_JAE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JB:
				Emit_JB(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JBE:
				Emit_JBE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JC:
				Emit_JC(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JCXZ:
				Emit_JCXZ(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JE:
				Emit_JE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JECXZ:
				Emit_JECXZ(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JG:
				Emit_JG(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JGE:
				Emit_JGE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JL:
				Emit_JL(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JLE:
				Emit_JLE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JMP:
				Emit_JMP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JNA:
				Emit_JNA(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JNAE:
				Emit_JNAE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JNB:
				Emit_JNB(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JNBE:
				Emit_JNBE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JNC:
				Emit_JNC(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JNE:
				Emit_JNE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JNG:
				Emit_JNG(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JNGE:
				Emit_JNGE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JNL:
				Emit_JNL(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JNLE:
				Emit_JNLE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JNO:
				Emit_JNO(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JNP:
				Emit_JNP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JNS:
				Emit_JNS(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JNZ:
				Emit_JNZ(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JO:
				Emit_JO(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JP:
				Emit_JP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JPE:
				Emit_JPE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JPO:
				Emit_JPO(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JS:
				Emit_JS(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::JZ:
				Emit_JZ(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::LEA:
				Emit_LEA(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::LODSB:
				Emit_LODSB(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::LODSD:
				Emit_LODSD(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::LODSW:
				Emit_LODSW(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::LOOP:
				Emit_LOOP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::LOOPE:
				Emit_LOOPE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::LOOPNE:
				Emit_LOOPNE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::LOOPNZ:
				Emit_LOOPNZ(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::LOOPZ:
				Emit_LOOPZ(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::MOV:
				Emit_MOV(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::MOVSB:
				Emit_MOVSB(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::MOVSD:
				Emit_MOVSD(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::MOVSW:
				Emit_MOVSW(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::MOVZX:
				Emit_MOVZX(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::MUL:
				Emit_MUL(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::NEG:
				Emit_NEG(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::NOT:
				Emit_NOT(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::OR:
				Emit_OR(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::POP:
				Emit_POP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::POPA:
				Emit_POPA(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::POPAD:
				Emit_POPAD(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::PUSH:
				Emit_PUSH(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::PUSHA:
				Emit_PUSHA(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::PUSHAD:
				Emit_PUSHAD(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::RCL:
				Emit_RCL(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::RCR:
				Emit_RCR(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::RET:
				Emit_RET(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::ROL:
				Emit_ROL(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::ROR:
				Emit_ROR(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SAHF:
				Emit_SAHF(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SAL:
				Emit_SAL(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SAR:
				Emit_SAR(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SBB:
				Emit_SBB(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SCASB:
				Emit_SCASB(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SCASD:
				Emit_SCASD(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SCASW:
				Emit_SCASW(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETA:
				Emit_SETA(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETAE:
				Emit_SETAE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETB:
				Emit_SETB(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETBE:
				Emit_SETBE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETC:
				Emit_SETC(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETE:
				Emit_SETE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETG:
				Emit_SETG(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETGE:
				Emit_SETGE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETL:
				Emit_SETL(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETLE:
				Emit_SETLE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETNA:
				Emit_SETNA(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETNAE:
				Emit_SETNAE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETNB:
				Emit_SETNB(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETNBE:
				Emit_SETNBE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETNC:
				Emit_SETNC(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETNE:
				Emit_SETNE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETNG:
				Emit_SETNG(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETNGE:
				Emit_SETNGE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETNL:
				Emit_SETNL(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETNLE:
				Emit_SETNLE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETNO:
				Emit_SETNO(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETNP:
				Emit_SETNP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETNS:
				Emit_SETNS(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETNZ:
				Emit_SETNZ(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETO:
				Emit_SETO(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETP:
				Emit_SETP(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETPE:
				Emit_SETPE(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETPO:
				Emit_SETPO(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETS:
				Emit_SETS(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SETZ:
				Emit_SETZ(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SHL:
				Emit_SHL(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SHR:
				Emit_SHR(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::STC:
				Emit_STC(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::STD:
				Emit_STD(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::STI:
				Emit_STI(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::STOSB:
				Emit_STOSB(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::STOSD:
				Emit_STOSD(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::STOSW:
				Emit_STOSW(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::SUB:
				Emit_SUB(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::TEST:
				Emit_TEST(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::XCHG:
				Emit_XCHG(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			case Instruction::XOR:
				Emit_XOR(instr.OperandCount, instr.Op1, instr.Op2);
				break;
			}
		}
	}
}


using namespace Compiler::x86;
typedef  int (__stdcall *JITFunc)(int a, int b);

void TestJIT()
{
	BinaryCodeEmitter emitter;
	emitter.Emit_PUSH(1, Operand(Register::EBP), Operand());
	emitter.Emit_MOV(2, Operand(Register::EBP), Operand(Register::ESP));
	emitter.Emit_SUB(2, Operand(Register::ESP), Operand(4, true));
	emitter.EmitLoad(Register::EAX, 8); // param1
	emitter.EmitLoad(Register::ECX, 12); // param2
	emitter.Emit_ADD(2, Operand(Register::EAX), Operand(Register::ECX));
	emitter.EmitStore(Register::EAX, -4); // var1
	emitter.EmitLoad(Register::ECX, -4);
	emitter.Emit_MOV(2, Operand(Register::ESP), Operand(Register::EBP));
	emitter.Emit_POP(1, Operand(Register::EBP), Operand());
	emitter.Emit_RET(1, Operand((unsigned short) 8), Operand());
	JITFunc f = (JITFunc)emitter.ToFunc();
	int value = f(100,2);
	printf("return value is %d\n", value);
	VirtualFree(f, 0, MEM_RELEASE);
}


