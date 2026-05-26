#ifndef ASSEMBLY_X86_H
#define ASSEMBLY_X86_H

#include <cstring>
#include <filesystem>
#include <list>
#include <string>
#include <vector>
#include <Windows.h>

namespace Compiler
{
	namespace x86
	{
		enum class Register
		{
			None = -1,
			EAX = 0,
			ECX = 1,
			EDX = 2,
			EBX = 3,
			ESP = 4,
			EBP = 5,
			ESI = 6,
			EDI = 7
		};
		inline int GetRegisterCode(Register reg)
		{
			return (int)reg;
		}
		class InvalidProgramException
		{
		public:
			const wchar_t * Message;
			InvalidProgramException(const wchar_t * message)
			{
				Message = message;
			}
		};
		enum class OperandType
		{
			None, Register, Immediate32, Immediate16, Immediate8,
			Memory, ST
		};
		enum class OperandAddressingMode
		{
			RegisterPointer, // can only be EAX, ECX, EDX, EBX
			SIB, Disp32, 
			RegisterDisp8, // can only be EAX, ECX, EDX, EBX, EBP, ESI, EDI
			SIBDisp8,
			RegisterDisp32, // can only be EAX, ECX, EDX, EBX, EBP, ESI, EDI
			SIBDisp32
		};
		enum class MemoryOperandType
		{
			Int8, Int16, Int32, Int64,
			Float32, Float64
		};
		class Operand
		{
		public:
			int SizeBits;
			Register Reg, IndexReg;
			unsigned char Scale;
			OperandType Type;
			MemoryOperandType MemoryType;
			OperandAddressingMode AddressingMode;
			unsigned int Value;
			bool IsIntImmediate() const
			{
				return Type == OperandType::Immediate32 || Type == OperandType::Immediate16 ||
					Type == OperandType::Immediate8;
			}
			bool operator == (const Operand & op) const 
			{
				return Reg == op.Reg && Type == op.Type && MemoryType == op.MemoryType &&
					AddressingMode == op.AddressingMode;
			}

			bool operator != (const Operand & op) const
			{
				return ! this->operator==(op);
			}

			Operand()
				:Type(OperandType::None), Value(0), SizeBits(0)
			{}
			Operand(int immediate32, bool autoDegrade)
			{
				Value = immediate32;
				if (autoDegrade)
				{
					if (immediate32 >= CHAR_MIN && immediate32 <= CHAR_MAX)
					{
						Type = OperandType::Immediate8;
						SizeBits = 8;
					}
					else
					{
						Type = OperandType::Immediate32;
						SizeBits = 32;
					}
				}
				else
				{
					Type = OperandType::Immediate32;
					SizeBits = 32;
				}
				
			}
			Operand(unsigned int immediate32)
			{
				Type = OperandType::Immediate32;
				Value = immediate32;
				SizeBits = 32;
			}
			Operand(unsigned short immediate16)
			{
				Type = OperandType::Immediate16;
				Value = immediate16;
				SizeBits = 16;
			}
			Operand(unsigned char immediate8)
			{
				Type = OperandType::Immediate8;
				Value = immediate8;
				SizeBits = 8;
			}
			Operand(Register reg, int bits = 32)
			{
				Type = OperandType::Register;
				Reg = reg;
				SizeBits = bits;
				if (reg == Register::None)
					throw InvalidProgramException(L"Illegal register");
			}
			static Operand FromST(int index)
			{
				Operand rs;
				rs.Type = OperandType::ST;
				rs.Value = index;
				rs.SizeBits = 64;
				return rs;
			}
			static Operand FromMemory(MemoryOperandType type, int displacement);
			// [baseReg + displacement]
			static Operand FromMemory(MemoryOperandType type, Register reg, int displacement);
			// [baseReg + indexReg*scale + displacement]
			static Operand FromMemory(MemoryOperandType type, Register baseReg, Register indexReg, int scale, int displacement);
			unsigned char GetSIB();
			void GetOpCode(unsigned char &rm, unsigned char & mod, bool & hasSIB, unsigned char & sib, int & postSize, unsigned char postBytes[4]);
		};

		class Instruction
		{
		public:
			enum InstructionName
			{
				ADC, ADD, AND, BOUND, BSF, BSR, BSWAP, BT, BTC, BTR, 
				BTS, CALL, CBW, CDQ, CLC, CLD, CLI, CMC, CMOVA, CMOVAE, 
				CMOVB, CMOVBE, CMOVC, CMOVE, CMOVG, CMOVGE, CMOVL, 
				CMOVLE, CMOVNA, CMOVNAE, CMOVNB, CMOVNBE, CMOVNC, CMOVNE, 
				CMOVNG, CMOVNGE, CMOVNL, CMOVNLE, CMOVNO, CMOVNP, CMOVNS, 
				CMOVNZ, CMOVO, CMOVP, CMOVPE, CMOVPO, CMOVS, CMOVZ, CMP, 
				CMPSB, CMPSD, CMPSW, CWD, CWDE, DEC, DIV, F2XM1, FABS, 
				FADD, FADDP, FCHS, FCMOVB, FCMOVBE, FCMOVE, FCMOVNB, 
				FCMOVNBE, FCMOVNE, FCMOVNU, FCMOVU, FCOM, FCOMI, FCOMIP, 
				FCOMP, FCOMPP, FCOS, FDECSTP, FDIV, FDIVP, FDIVR, FDIVRP,
				FIADD, FICOM, FICOMP, FIDIV, FIDIVR, FILD, FIMUL, FINCSTP,
				FINIT, FIST, FISTP, FISTTP, FISUB, FISUBR, FLD, FLD1, 
				FLDL2E, FLDL2T, FLDLG2, FLDLN2, FLDPI, FLDZ, FMUL, FMULP, 
				FNINIT, FNSTSW, FPATAN, FRNDINT, FSCALE, FSIN, FSINCOS, 
				FSQRT, FST, FSTP, FSTSW, FSUB, FSUBP, FSUBR, FSUBRP, FTST,
				FUCOM, FUCOMI, FUCOMIP, FUCOMP, FUCOMPP, FXAM, FXCH, 
				FXTRACT, FYL2X, FYL2XP1, IDIV, IMUL, INC, INT, INTO, JA, 
				JAE, JB, JBE, JC, JCXZ, JE, JECXZ, JG, JGE, JL, JLE, JMP, 
				JNA, JNAE, JNB, JNBE, JNC, JNE, JNG, JNGE, JNL, JNLE, JNO,
				JNP, JNS, JNZ, JO, JP, JPE, JPO, JS, JZ, LEA, LODSB, LODSD,
				LODSW, LOOP, LOOPE, LOOPNE, LOOPNZ, LOOPZ, MOV, MOVSB, 
				MOVSD, MOVSW, MOVZX, MUL, NEG, NOT, OR, POP, POPA, POPAD, 
				PUSH, PUSHA, PUSHAD, RCL, RCR, RET, ROL, ROR, SAHF, SAL, 
				SAR, SBB, SCASB, SCASD, SCASW, SETA, SETAE, SETB, SETBE, 
				SETC, SETE, SETG, SETGE, SETL, SETLE, SETNA, SETNAE, SETNB,
				SETNBE, SETNC, SETNE, SETNG, SETNGE, SETNL, SETNLE, SETNO,
				SETNP, SETNS, SETNZ, SETO, SETP, SETPE, SETPO, SETS, SETZ,
				SHL, SHR, STC, STD, STI, STOSB, STOSD, STOSW, SUB, TEST, 
				XCHG, XOR, UNKNOWN
			};
			InstructionName Name;
			Operand Op1, Op2;
			int OperandCount;
			int Label;
			Instruction() {};
			Instruction(InstructionName op)
			{
				OperandCount = 0;
				Label = 0;
				Name = op;
			}
			Instruction(InstructionName op, const Operand & op1)
			{
				Label = 0;
				OperandCount = 1;
				Op1 = op1;
				Name = op;
			}
			Instruction(InstructionName op, const Operand & op1, const Operand & op2)
			{
				Label = 0;
				OperandCount = 2;
				Op2 = op2;
				Op1 = op1;
				Name = op;
			}
			std::wstring ToString() const;
			bool IsJump()
			{
				switch (Name)
				{
				case JA:
				case JAE:
				case JB:
				case JBE:
				case JC:
				case JCXZ:
				case JE:
				case JECXZ:
				case JG:
				case JGE:
				case JL:
				case JLE:
				case JMP:
				case JNA:
				case JNAE:
				case JNB:
				case JNBE:
				case JNC:
				case JNE:
				case JNG:
				case JNGE:
				case JNL:
				case JNLE:
				case JNO:
				case JNP:
				case JNS:
				case JNZ:
				case JO:
				case JP:
				case JPE:
				case JPO:
				case JS:
				case JZ:
					return true;
				default:
					return false;
				}
			}
		};

		struct FunctionLinkPoint
		{
			int FuncId;
			int Position;
			FunctionLinkPoint()
				: FuncId(-1), Position(0)
			{}
			FunctionLinkPoint(int funcId, int pos)
				: FuncId(funcId), Position(pos)
			{}
		};

		enum class FloatType
		{
			Float, Double
		};

		class FloatConstant
		{
		public:
			FloatType Type;
			union
			{
				double DoubleValue;
				float FloatValue;
			};
		};

		class Function_x86
		{
		private:
			Function_x86(const Function_x86 & f){}
		public:
			std::list<Instruction> Code;
			std::wstring Name;
			std::vector<FloatConstant> FloatConsts;
			Function_x86(){}
			
			Function_x86(Function_x86 && f)
			{
				this->operator=(std::move(f));
			}
			Function_x86 & operator = (Function_x86 && f)
			{
				Code = std::move(f.Code);
				Name = std::move(f.Name);
				FloatConsts = std::move(f.FloatConsts);
				return *this;
			}

			// add const & version operator=
			//用默认的足够吗？复制构造是空的还是私有，移动构造也只是move了一下
			//这里是我添加的部分.
			Function_x86& operator = (const Function_x86& f) = default;

			void Dump(const std::filesystem::path & fileName);
		};

		class MemoryExecutable_x86
		{
		private:
			void Free()
			{
				if (Buffer)
					VirtualFree(Buffer, 0, MEM_RELEASE);
				Buffer = 0;
				BufferSize = 0;
			}
		public:
			void * Buffer;
			int BufferSize;
			MemoryExecutable_x86()
				: Buffer(0), BufferSize(0)
			{}
			~MemoryExecutable_x86()
			{
				Free();
			}
			MemoryExecutable_x86(const MemoryExecutable_x86 & exe)
				: Buffer(0), BufferSize(0)
			{
				this->operator=(exe);
			}
			MemoryExecutable_x86(MemoryExecutable_x86 && exe)
				: Buffer(0), BufferSize(0)
			{
				this->operator=(std::move(exe));
			}

			MemoryExecutable_x86 & operator=(MemoryExecutable_x86 && exe)
			{
				Free();
				Buffer = exe.Buffer;
				BufferSize = exe.BufferSize;
				exe.Buffer = 0;
				exe.BufferSize = 0;
				return *this;
			}

			MemoryExecutable_x86 & operator=(const MemoryExecutable_x86 & exe)
			{
				Free();
				if (exe.Buffer)
				{
					Buffer = VirtualAlloc(0, exe.BufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
					std::memcpy(Buffer, exe.Buffer, exe.BufferSize);
					BufferSize = exe.BufferSize;
				}
				return *this;
			}
		};

		class Assembly_x86	//明显是
		{
		public:
			std::vector<unsigned char> ConstBuffer;
			std::vector<unsigned char> CodeBuffer;
			std::vector<FunctionLinkPoint> FunctionPointerLinkPoints;
			std::vector<int> ConstantLinkPoints;
			MemoryExecutable_x86 CreateMemoryExecutable();
			template<typename T>
			int AddConstant(T val)
			{
				int rs = static_cast<int>(ConstBuffer.size());
				auto oldSize = ConstBuffer.size();
				ConstBuffer.resize(oldSize + sizeof(val));
				std::memcpy(ConstBuffer.data() + oldSize, &val, sizeof(val));
				return rs;
			}
		};

		class Program_x86
		{
		public:
			std::vector<Function_x86> Functions;
			Assembly_x86 Link();
		};
	}
}

#endif
