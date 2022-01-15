#include "Assembly_x86.h"
#include "CodeEmitter_x86.h"
#include "LibIO.h"

using namespace CoreLib::IO;
namespace Compiler
{
	namespace x86
	{
		struct ConstantMap
		{
			Dictionary<double, int> DoubleValues;
			Dictionary<float, int> FloatValues;
		};

		wchar_t * InstructionNameToString(Instruction::InstructionName name);

		void RegToString(StringBuilder & sb, Register reg)
		{
		switch (reg)
			{
			case Register::EAX:
				sb.Append(L"EAX");
				break;
			case Register::EBX:
				sb.Append(L"EBX");
				break;
			case Register::ECX:
				sb.Append(L"ECX");
				break;
			case Register::EDX:
				sb.Append(L"EDX");
				break;
			case Register::ESP:
				sb.Append(L"ESP");
				break;
			case Register::EBP:
				sb.Append(L"EBP");
				break;
			case Register::ESI:
				sb.Append(L"ESI");
				break;
			case Register::EDI:
				sb.Append(L"EDI");
				break;
			}
		}

		void OperandToString(StringBuilder & sb, Operand & op)
		{
			switch (op.Type)
			{
			case OperandType::Register:
				RegToString(sb, op.Reg);
				break;
			case OperandType::ST:
				sb.Append(L"ST");
				sb.Append((int)op.Value);
				break;
			case OperandType::Immediate8:
			case OperandType::Immediate16:
			case OperandType::Immediate32:
				sb.Append((int)op.Value);
				break;
			case OperandType::Memory:
				sb.Append(L"ptr[");
				if (op.AddressingMode == OperandAddressingMode::Disp32)
				{
					sb.Append((int)op.Value);
				}
				else if (op.AddressingMode == OperandAddressingMode::RegisterDisp32 ||
					op.AddressingMode == OperandAddressingMode::RegisterDisp8)
				{
					RegToString(sb, op.Reg);
					if (op.Value != 0)
					{
						sb.Append(L"+");
						sb.Append((int)op.Value);
					}
				}
				else if (op.AddressingMode == OperandAddressingMode::RegisterPointer)
				{
					RegToString(sb, op.Reg);
				}
				else if (op.AddressingMode == OperandAddressingMode::SIB)
				{
					RegToString(sb, op.Reg);
					if (op.IndexReg != Register::None)
					{
						sb.Append(L"+");
						RegToString(sb, op.IndexReg);
						sb.Append(L"*");
						sb.Append((int)op.Scale);
					}
				}
				else if (op.AddressingMode == OperandAddressingMode::SIBDisp32 ||
					op.AddressingMode == OperandAddressingMode::SIBDisp8)
				{
					RegToString(sb, op.Reg);
					if (op.IndexReg != Register::None)
					{
						sb.Append(L"+");
						RegToString(sb, op.IndexReg);
						sb.Append(L"*");
						sb.Append((int)op.Scale);
					}
					if (op.Value != 0)
					{
						sb.Append(L"+");
						sb.Append((int)op.Value);
					}
				}
				sb.Append(L']');
				break;
			}
		}

		String Instruction::ToString()
		{
			StringBuilder sb;
			sb.Append(Label);
			sb.Append(L":\t");
			sb.Append(InstructionNameToString(Name));
			sb.Append(L'\t');
			if (OperandCount > 0)
				OperandToString(sb, Op1);
			if (OperandCount > 1)
			{
				sb.Append(L',');
				OperandToString(sb, Op2);
			}
			return sb.ProduceString();
		}

		void Function_x86::Dump(const String & fileName)
		{
			StreamWriter writer(fileName);
			for (auto & instr : Code)
			{
				writer<<instr.ToString()<<EndLine;
			}
		}

		void LinkFunction(int funcId, ConstantMap & consts, Assembly_x86 & assembly, Function_x86 & func)
		{
			FunctionMeta meta;
			meta.Name = func.Name;
			meta.Id = funcId;
			meta.Offset = assembly.CodeBuffer.Count();
			assembly.Functions.Add(meta);
			BinaryCodeEmitter emitter;
			Dictionary<int, int> labelMap;
			List<int> labels;
			for (auto & instr : func.Code)
			{
				if (instr.IsJump())
					labelMap[instr.Op1.Value] = -1;
			}
			for (auto & instr : func.Code)
			{
				if (labelMap.ContainsKey(instr.Label))
				{
					labelMap[instr.Label] = emitter.GetCode().Count();
				}
				emitter.Emit(instr);
				if (instr.IsJump())
					labels.Add(emitter.GetCode().Count()-4);
				if (instr.Name == Instruction::CALL && instr.Op1.Type == OperandType::Immediate32)
					assembly.FunctionPointerLinkPoints.Add(FunctionLinkPoint(instr.Op1.Value, emitter.GetCode().Count()-4+assembly.CodeBuffer.Count()));
				if (instr.Name == Instruction::FLD && instr.Op1.Type == x86::OperandType::Memory && instr.Op1.AddressingMode == OperandAddressingMode::Disp32)
				{
					int constId = instr.Op1.Value;
					auto c = func.FloatConsts[constId];
					int newPosVal = 0;
					if (c.Type == FloatType::Float)
					{
						if (!consts.FloatValues.TryGetValue(c.FloatValue, newPosVal))
						{
							newPosVal = assembly.AddConstant(c.FloatValue);
							consts.FloatValues[c.FloatValue] = newPosVal;
						}
					}
					else
					{
						if (!consts.DoubleValues.TryGetValue(c.DoubleValue, newPosVal))
						{
							newPosVal = assembly.AddConstant(c.DoubleValue);
							consts.DoubleValues[c.DoubleValue] = newPosVal;
						}
					}
					assembly.ConstantLinkPoints.Add(emitter.GetCode().Count()-4+assembly.CodeBuffer.Count());
					auto & code = emitter.GetCode();
					*(int*)(code.Buffer()-4) = newPosVal;
				}
			}
			auto & code = emitter.GetCode();
			for (int pos : labels)
			{
				int * pLabel = (int*)(code.Buffer() + pos);
				*pLabel = labelMap[*pLabel] - pos - 4;
			}
			assembly.CodeBuffer.AddRange(code);
		}

		//把代码里的主函数链接到可执行文件里？
		Assembly_x86 Program_x86::Link()
		{
			Assembly_x86 rs;
			Function_x86 * mainFunc = 0;
			ConstantMap constMapping;
			Dictionary<int, int> funcPositions;
			int mainFuncId = 0;
			for (auto & func : Functions)
			{
				if (func.Name == L"main")
				{
					mainFunc = &func;
					break;
				}
				mainFuncId++;
			}
			LinkFunction(mainFuncId, constMapping, rs, *mainFunc);
			funcPositions[mainFuncId] = 0;
			int id = 0;
			for (auto & func : Functions)
			{
				if (&func != mainFunc)
				{
					funcPositions[id] = rs.CodeBuffer.Count();
					LinkFunction(id, constMapping, rs, func);
				}
				id++;
			}
			for (auto & link : rs.FunctionPointerLinkPoints)
			{
				int * pFunc = (int*)(rs.CodeBuffer.Buffer() + link.Position);
				*pFunc = funcPositions[link.FuncId] - link.Position - 4;
			}
			return rs;
		}

		MemoryExecutable_x86 Assembly_x86::CreateMemoryExecutable()	//在内存里创建可执行，然后把可执行代码写进去。
		{
			MemoryExecutable_x86 rs;
			rs.BufferSize = CodeBuffer.Count() + ConstBuffer.Count();

			//在调用进程的虚地址空间,预定或者提交一部分页。
			rs.Buffer = VirtualAlloc(0, rs.BufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

			memcpy(rs.Buffer, CodeBuffer.Buffer(), CodeBuffer.Count());
			char * pConst = (char*)rs.Buffer + CodeBuffer.Count();
			memcpy(pConst, ConstBuffer.Buffer(), ConstBuffer.Count());
			// runtime linking
			/*for (auto flink : FunctionPointerLinkPoints)
			{
				int * ptr = (int*)((char*)rs.Buffer + flink.Position);
				*ptr = *ptr + (int)(rs.Buffer);
			}*/
			for (auto clink : ConstantLinkPoints)
			{
				int * ptr = (int*)((char*)rs.Buffer + clink);
				*ptr = *ptr + (int)(pConst);
			}
			return rs;
		}

		wchar_t * InstructionNameToString(Instruction::InstructionName name)	//全都是x64指令
		{
			switch (name)
			{
			case Instruction::ADC:
				return L"ADC";
			case Instruction::ADD:
				return L"ADD";
			case Instruction::AND:
				return L"AND";
			case Instruction::BOUND:
				return L"BOUND";
			case Instruction::BSF:
				return L"BSF";
			case Instruction::BSR:
				return L"BSR";
			case Instruction::BSWAP:
				return L"BSWAP";
			case Instruction::BT:
				return L"BT";
			case Instruction::BTC:
				return L"BTC";
			case Instruction::BTR:
				return L"BTR";
			case Instruction::BTS:
				return L"BTS";
			case Instruction::CALL:
				return L"CALL";
			case Instruction::CBW:
				return L"CBW";
			case Instruction::CDQ:
				return L"CDQ";
			case Instruction::CLC:
				return L"CLC";
			case Instruction::CLD:
				return L"CLD";
			case Instruction::CLI:
				return L"CLI";
			case Instruction::CMC:
				return L"CMC";
			case Instruction::CMOVA:
				return L"CMOVA";
			case Instruction::CMOVAE:
				return L"CMOVAE";
			case Instruction::CMOVB:
				return L"CMOVB";
			case Instruction::CMOVBE:
				return L"CMOVBE";
			case Instruction::CMOVC:
				return L"CMOVC";
			case Instruction::CMOVE:
				return L"CMOVE";
			case Instruction::CMOVG:
				return L"CMOVG";
			case Instruction::CMOVGE:
				return L"CMOVGE";
			case Instruction::CMOVL:
				return L"CMOVL";
			case Instruction::CMOVLE:
				return L"CMOVLE";
			case Instruction::CMOVNA:
				return L"CMOVNA";
			case Instruction::CMOVNAE:
				return L"CMOVNAE";
			case Instruction::CMOVNB:
				return L"CMOVNB";
			case Instruction::CMOVNBE:
				return L"CMOVNBE";
			case Instruction::CMOVNC:
				return L"CMOVNC";
			case Instruction::CMOVNE:
				return L"CMOVNE";
			case Instruction::CMOVNG:
				return L"CMOVNG";
			case Instruction::CMOVNGE:
				return L"CMOVNGE";
			case Instruction::CMOVNL:
				return L"CMOVNL";
			case Instruction::CMOVNLE:
				return L"CMOVNLE";
			case Instruction::CMOVNO:
				return L"CMOVNO";
			case Instruction::CMOVNP:
				return L"CMOVNP";
			case Instruction::CMOVNS:
				return L"CMOVNS";
			case Instruction::CMOVNZ:
				return L"CMOVNZ";
			case Instruction::CMOVO:
				return L"CMOVO";
			case Instruction::CMOVP:
				return L"CMOVP";
			case Instruction::CMOVPE:
				return L"CMOVPE";
			case Instruction::CMOVPO:
				return L"CMOVPO";
			case Instruction::CMOVS:
				return L"CMOVS";
			case Instruction::CMOVZ:
				return L"CMOVZ";
			case Instruction::CMP:
				return L"CMP";
			case Instruction::CMPSB:
				return L"CMPSB";
			case Instruction::CMPSD:
				return L"CMPSD";
			case Instruction::CMPSW:
				return L"CMPSW";
			case Instruction::CWD:
				return L"CWD";
			case Instruction::CWDE:
				return L"CWDE";
			case Instruction::DEC:
				return L"DEC";
			case Instruction::DIV:
				return L"DIV";
			case Instruction::F2XM1:
				return L"F2XM1";
			case Instruction::FABS:
				return L"FABS";
			case Instruction::FADD:
				return L"FADD";
			case Instruction::FADDP:
				return L"FADDP";
			case Instruction::FCHS:
				return L"FCHS";
			case Instruction::FCMOVB:
				return L"FCMOVB";
			case Instruction::FCMOVBE:
				return L"FCMOVBE";
			case Instruction::FCMOVE:
				return L"FCMOVE";
			case Instruction::FCMOVNB:
				return L"FCMOVNB";
			case Instruction::FCMOVNBE:
				return L"FCMOVNBE";
			case Instruction::FCMOVNE:
				return L"FCMOVNE";
			case Instruction::FCMOVNU:
				return L"FCMOVNU";
			case Instruction::FCMOVU:
				return L"FCMOVU";
			case Instruction::FCOM:
				return L"FCOM";
			case Instruction::FCOMI:
				return L"FCOMI";
			case Instruction::FCOMIP:
				return L"FCOMIP";
			case Instruction::FCOMP:
				return L"FCOMP";
			case Instruction::FCOMPP:
				return L"FCOMPP";
			case Instruction::FCOS:
				return L"FCOS";
			case Instruction::FDECSTP:
				return L"FDECSTP";
			case Instruction::FDIV:
				return L"FDIV";
			case Instruction::FDIVP:
				return L"FDIVP";
			case Instruction::FDIVR:
				return L"FDIVR";
			case Instruction::FDIVRP:
				return L"FDIVRP";
			case Instruction::FIADD:
				return L"FIADD";
			case Instruction::FICOM:
				return L"FICOM";
			case Instruction::FICOMP:
				return L"FICOMP";
			case Instruction::FIDIV:
				return L"FIDIV";
			case Instruction::FIDIVR:
				return L"FIDIVR";
			case Instruction::FILD:
				return L"FILD";
			case Instruction::FIMUL:
				return L"FIMUL";
			case Instruction::FINCSTP:
				return L"FINCSTP";
			case Instruction::FINIT:
				return L"FINIT";
			case Instruction::FIST:
				return L"FIST";
			case Instruction::FISTP:
				return L"FISTP";
			case Instruction::FISTTP:
				return L"FISTTP";
			case Instruction::FISUB:
				return L"FISUB";
			case Instruction::FISUBR:
				return L"FISUBR";
			case Instruction::FLD:
				return L"FLD";
			case Instruction::FLD1:
				return L"FLD1";
			case Instruction::FLDL2E:
				return L"FLDL2E";
			case Instruction::FLDL2T:
				return L"FLDL2T";
			case Instruction::FLDLG2:
				return L"FLDLG2";
			case Instruction::FLDLN2:
				return L"FLDLN2";
			case Instruction::FLDPI:
				return L"FLDPI";
			case Instruction::FLDZ:
				return L"FLDZ";
			case Instruction::FMUL:
				return L"FMUL";
			case Instruction::FMULP:
				return L"FMULP";
			case Instruction::FNINIT:
				return L"FNINIT";
			case Instruction::FNSTSW:
				return L"FNSTSW";
			case Instruction::FPATAN:
				return L"FPATAN";
			case Instruction::FRNDINT:
				return L"FRNDINT";
			case Instruction::FSCALE:
				return L"FSCALE";
			case Instruction::FSIN:
				return L"FSIN";
			case Instruction::FSINCOS:
				return L"FSINCOS";
			case Instruction::FSQRT:
				return L"FSQRT";
			case Instruction::FST:
				return L"FST";
			case Instruction::FSTP:
				return L"FSTP";
			case Instruction::FSTSW:
				return L"FSTSW";
			case Instruction::FSUB:
				return L"FSUB";
			case Instruction::FSUBP:
				return L"FSUBP";
			case Instruction::FSUBR:
				return L"FSUBR";
			case Instruction::FSUBRP:
				return L"FSUBRP";
			case Instruction::FTST:
				return L"FTST";
			case Instruction::FUCOM:
				return L"FUCOM";
			case Instruction::FUCOMI:
				return L"FUCOMI";
			case Instruction::FUCOMIP:
				return L"FUCOMIP";
			case Instruction::FUCOMP:
				return L"FUCOMP";
			case Instruction::FUCOMPP:
				return L"FUCOMPP";
			case Instruction::FXAM:
				return L"FXAM";
			case Instruction::FXCH:
				return L"FXCH";
			case Instruction::FXTRACT:
				return L"FXTRACT";
			case Instruction::FYL2X:
				return L"FYL2X";
			case Instruction::FYL2XP1:
				return L"FYL2XP1";
			case Instruction::IDIV:
				return L"IDIV";
			case Instruction::IMUL:
				return L"IMUL";
			case Instruction::INC:
				return L"INC";
			case Instruction::INT:
				return L"INT";
			case Instruction::INTO:
				return L"INTO";
			case Instruction::JA:
				return L"JA";
			case Instruction::JAE:
				return L"JAE";
			case Instruction::JB:
				return L"JB";
			case Instruction::JBE:
				return L"JBE";
			case Instruction::JC:
				return L"JC";
			case Instruction::JCXZ:
				return L"JCXZ";
			case Instruction::JE:
				return L"JE";
			case Instruction::JECXZ:
				return L"JECXZ";
			case Instruction::JG:
				return L"JG";
			case Instruction::JGE:
				return L"JGE";
			case Instruction::JL:
				return L"JL";
			case Instruction::JLE:
				return L"JLE";
			case Instruction::JMP:
				return L"JMP";
			case Instruction::JNA:
				return L"JNA";
			case Instruction::JNAE:
				return L"JNAE";
			case Instruction::JNB:
				return L"JNB";
			case Instruction::JNBE:
				return L"JNBE";
			case Instruction::JNC:
				return L"JNC";
			case Instruction::JNE:
				return L"JNE";
			case Instruction::JNG:
				return L"JNG";
			case Instruction::JNGE:
				return L"JNGE";
			case Instruction::JNL:
				return L"JNL";
			case Instruction::JNLE:
				return L"JNLE";
			case Instruction::JNO:
				return L"JNO";
			case Instruction::JNP:
				return L"JNP";
			case Instruction::JNS:
				return L"JNS";
			case Instruction::JNZ:
				return L"JNZ";
			case Instruction::JO:
				return L"JO";
			case Instruction::JP:
				return L"JP";
			case Instruction::JPE:
				return L"JPE";
			case Instruction::JPO:
				return L"JPO";
			case Instruction::JS:
				return L"JS";
			case Instruction::JZ:
				return L"JZ";
			case Instruction::LEA:
				return L"LEA";
			case Instruction::LODSB:
				return L"LODSB";
			case Instruction::LODSD:
				return L"LODSD";
			case Instruction::LODSW:
				return L"LODSW";
			case Instruction::LOOP:
				return L"LOOP";
			case Instruction::LOOPE:
				return L"LOOPE";
			case Instruction::LOOPNE:
				return L"LOOPNE";
			case Instruction::LOOPNZ:
				return L"LOOPNZ";
			case Instruction::LOOPZ:
				return L"LOOPZ";
			case Instruction::MOV:
				return L"MOV";
			case Instruction::MOVSB:
				return L"MOVSB";
			case Instruction::MOVSD:
				return L"MOVSD";
			case Instruction::MOVSW:
				return L"MOVSW";
			case Instruction::MOVZX:
				return L"MOVZX";
			case Instruction::MUL:
				return L"MUL";
			case Instruction::NEG:
				return L"NEG";
			case Instruction::NOT:
				return L"NOT";
			case Instruction::OR:
				return L"OR";
			case Instruction::POP:
				return L"POP";
			case Instruction::POPA:
				return L"POPA";
			case Instruction::POPAD:
				return L"POPAD";
			case Instruction::PUSH:
				return L"PUSH";
			case Instruction::PUSHA:
				return L"PUSHA";
			case Instruction::PUSHAD:
				return L"PUSHAD";
			case Instruction::RCL:
				return L"RCL";
			case Instruction::RCR:
				return L"RCR";
			case Instruction::RET:
				return L"RET";
			case Instruction::ROL:
				return L"ROL";
			case Instruction::ROR:
				return L"ROR";
			case Instruction::SAHF:
				return L"SAHF";
			case Instruction::SAL:
				return L"SAL";
			case Instruction::SAR:
				return L"SAR";
			case Instruction::SBB:
				return L"SBB";
			case Instruction::SCASB:
				return L"SCASB";
			case Instruction::SCASD:
				return L"SCASD";
			case Instruction::SCASW:
				return L"SCASW";
			case Instruction::SETA:
				return L"SETA";
			case Instruction::SETAE:
				return L"SETAE";
			case Instruction::SETB:
				return L"SETB";
			case Instruction::SETBE:
				return L"SETBE";
			case Instruction::SETC:
				return L"SETC";
			case Instruction::SETE:
				return L"SETE";
			case Instruction::SETG:
				return L"SETG";
			case Instruction::SETGE:
				return L"SETGE";
			case Instruction::SETL:
				return L"SETL";
			case Instruction::SETLE:
				return L"SETLE";
			case Instruction::SETNA:
				return L"SETNA";
			case Instruction::SETNAE:
				return L"SETNAE";
			case Instruction::SETNB:
				return L"SETNB";
			case Instruction::SETNBE:
				return L"SETNBE";
			case Instruction::SETNC:
				return L"SETNC";
			case Instruction::SETNE:
				return L"SETNE";
			case Instruction::SETNG:
				return L"SETNG";
			case Instruction::SETNGE:
				return L"SETNGE";
			case Instruction::SETNL:
				return L"SETNL";
			case Instruction::SETNLE:
				return L"SETNLE";
			case Instruction::SETNO:
				return L"SETNO";
			case Instruction::SETNP:
				return L"SETNP";
			case Instruction::SETNS:
				return L"SETNS";
			case Instruction::SETNZ:
				return L"SETNZ";
			case Instruction::SETO:
				return L"SETO";
			case Instruction::SETP:
				return L"SETP";
			case Instruction::SETPE:
				return L"SETPE";
			case Instruction::SETPO:
				return L"SETPO";
			case Instruction::SETS:
				return L"SETS";
			case Instruction::SETZ:
				return L"SETZ";
			case Instruction::SHL:
				return L"SHL";
			case Instruction::SHR:
				return L"SHR";
			case Instruction::STC:
				return L"STC";
			case Instruction::STD:
				return L"STD";
			case Instruction::STI:
				return L"STI";
			case Instruction::STOSB:
				return L"STOSB";
			case Instruction::STOSD:
				return L"STOSD";
			case Instruction::STOSW:
				return L"STOSW";
			case Instruction::SUB:
				return L"SUB";
			case Instruction::TEST:
				return L"TEST";
			case Instruction::XCHG:
				return L"XCHG";
			case Instruction::XOR:
				return L"XOR";
			default:
				return L"unkown";
			}
		}

	}
}