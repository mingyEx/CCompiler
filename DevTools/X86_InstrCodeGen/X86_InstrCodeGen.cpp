// X86_InstrCodeGen.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "VL_JIT_X86.h"
#include "Basic.h"
#include <algorithm>
using namespace vl::jit::x86;
using namespace CoreLib::Basic;

bool InsCompare(VLS_InsFormat i1, VLS_InsFormat i2)
{
	int r1 = i1.Rank();
	int r2 = i2.Rank();
	return (r1<r2);
}

int _tmain(int argc, _TCHAR* argv[])	//这个文件是干嘛的.. 意思是.. 用来生成?
{
	const int instrCount = 246;
	StringBuilder sb(4*1024*1024);
	StringBuilder header(1024*512);
	sb.Append(L"#include \"CodeEmitter_x86.h\"\n");
	sb.Append(L"namespace Compiler\n{\n");
	sb.Append(L"namespace x86\n{\n");
	for (int ins = 0; ins<instrCount; ins++)
	{
		auto name = InsName[ins];
		header.Append(L"\t\t\tvoid Emit_");
		header.Append(name);
		header.Append("(int paramCount, const Operand & op1, const Operand & op2);\n");
		sb.Append("void ");
		sb.Append(L"BinaryCodeEmitter::Emit_");
		sb.Append(name);
		sb.Append("(int paramCount, const Operand & op1, const Operand & op2)\n");
		sb.Append(L"{\n");
		std::sort(InsFormat+InsOffset[ins], InsFormat+InsOffset[ins]+InsCount[ins], InsCompare);
		for (int i = InsOffset[ins]; i<InsOffset[ins]+InsCount[ins]; i++)
		{

			int paramInclude[2];
			paramInclude[0] = 0; paramInclude[1] = 0;
			auto format = InsFormat[i];
			int paramCount = 0;
			if (format.Prefix16)
				continue;
			if (format.Params[0] == vipNoParam)
				paramCount = 0;
			else if (format.Params[1] == vipNoParam)
				paramCount = 1;
			else if (format.Params[2] == vipNoParam)
				paramCount = 2;
			else
			{
				paramCount = 3;
			}
			if (paramCount > 2)
				continue;

			bool shouldOmit = false;
			for (int p = 0; p<paramCount; p++)
				if (format.Params[p] == vipPTR_16_16 || format.Params[p] == vipPTR_16_32 
					|| format.Params[p] == vipM_16_16 || format.Params[p] == vipM_16_32 
					|| format.Params[p] == vipM_16_AND_16 || format.Params[p] == vipM_16_AND_32 
					|| format.Params[p] == vipM_32_AND_32 || format.Params[p] == vipMF_80 )
					shouldOmit = true;
			if (shouldOmit)
				continue;

			bool hasRMByte = false;
			int regs = 0;
			for (int p = 0; p<paramCount; p++)
			{
				if (format.Params[p] == vipRM_16 || format.Params[p] == vipRM_32 ||
					format.Params[p] == vipRM_8 || format.Params[p] == vipMF_32 ||
					format.Params[p] == vipMF_64 || format.Params[p] == vipMI_32 ||
					format.Params[p] == vipMI_64 || format.Params[p] == vipMI_16)
					hasRMByte = true;
				if (format.Params[p] == vipREG_8 || format.Params[p] == vipREG_16 ||
					format.Params[p] == vipREG_32)
					regs++;
			}
			if (format.Ext != vieNoExt)
				hasRMByte = true;
			if (format.Plus == vipRegister)
				regs--;
			if (regs)
				hasRMByte = true;

			int oldLen = sb.Length();
			// Match parameters
			sb.Append(L"if (paramCount == ");
			sb.Append(String(paramCount));
			for (int p = 0; p<paramCount; p++)
			{
				sb.Append(L" && (");
				String opName = String(L"op") + String(p+1);
				String targetParamCondition;
				switch (format.Params[p])
				{
				case vipREL_8:
				case vipIMM_8:
					targetParamCondition = opName + L".Type == OperandType::Immediate8";
					break;
				case vipREL_16:
				case vipIMM_16:
					targetParamCondition = opName + L".Type == OperandType::Immediate16";
					break;
				case vipREL_32:
				case vipIMM_32:
					targetParamCondition = opName + L".Type == OperandType::Immediate32";
					break;
				case vipREG_8:
					targetParamCondition = opName + L".SizeBits == 8 && " + opName + L".Type == OperandType::Register";
					break;
				case vipREG_16:	
					targetParamCondition = opName + L".SizeBits == 16 && " + opName + L".Type == OperandType::Register";
					break;
				case vipREG_32:
					targetParamCondition = opName + L".SizeBits == 32 && " + opName + L".Type == OperandType::Register";
					break;
				case vipRM_8:
					targetParamCondition = opName + L".SizeBits == 8 && (" + opName + L".Type == OperandType::Register || " + opName + L".Type == OperandType::Memory" + L")";
					break;
				case vipRM_16:
					targetParamCondition = opName + L".SizeBits == 16 && (" + opName + L".Type == OperandType::Register || " + opName + L".Type == OperandType::Memory" + L")";
					break;
				case vipRM_32:
					targetParamCondition = opName + L".SizeBits == 32 && (" + opName + L".Type == OperandType::Register || " + opName + L".Type == OperandType::Memory" + L")";
					break;
				case vipMI_16:
					targetParamCondition = opName + L".Type == OperandType::Memory && "  + opName + L".MemoryType == MemoryOperandType::Int16";
					break;
				case vipMI_32:
					targetParamCondition = opName + L".Type == OperandType::Memory && "  + opName + L".MemoryType == MemoryOperandType::Int32";
					break;
				case vipMI_64:
					targetParamCondition = opName + L".Type == OperandType::Memory && "  + opName + L".MemoryType == MemoryOperandType::Int64";
					break;
				case vipMF_32:
					targetParamCondition = opName + L".Type == OperandType::Memory && "  + opName + L".MemoryType == MemoryOperandType::Float32";
					break;
				case vipMF_64:
					targetParamCondition = opName + L".Type == OperandType::Memory && "  + opName + L".MemoryType == MemoryOperandType::Float64";
					break;
				case vipST_0:
					targetParamCondition = opName + L".Type == OperandType::ST && "  + opName + L".Value == 0";
					paramInclude[p]++;
					break;
				case vipST_I:
					targetParamCondition = opName + L".Type == OperandType::ST";
					break;
				case vipEAX:
					targetParamCondition = opName + L".Type == OperandType::Register && " + opName + L".Reg == Register::EAX";
					paramInclude[p]++;
					break;
				case vipECX:
				case vipCL:
				case vipCX:
					targetParamCondition = opName + L".Type == OperandType::Register && " + opName + L".Reg == Register::ECX";
					paramInclude[p]++;
					break;
				case vipCONST_1:
					targetParamCondition = L"(" + opName + L".Type == OperandType::Immediate32 || " + opName + L".Type == OperandType::Immediate8) && " + opName + L".Value == 1";
					paramInclude[p]++;
					break;
				case vipCONST_3:
					targetParamCondition = L"(" + opName + L".Type == OperandType::Immediate32 || " + opName + L".Type == OperandType::Immediate8) && " + opName + L".Value == 3";
					paramInclude[p]++;
					break;
				case vipCS:
				case vipDS:
				case vipES:
				case vipFS:
				case vipGS:
				case vipSS:
				case vipAL:
				case vipAX:
				case vipNoParam:
				case vipMF_80:
				case vipSREG:
				case vipPTR_16_16:
				case vipPTR_16_32:
				case vipM_16_16:
				case vipM_16_32:
				case vipM_16_AND_16:
				case vipM_16_AND_32:
				case vipM_32_AND_32:
				default:
					shouldOmit = true;
					targetParamCondition = "false";
					break;
				}
				sb.Append(targetParamCondition);
				sb.Append(L")");
			}
			if (shouldOmit)
			{
				sb.Remove(oldLen, sb.Length()-oldLen);
				continue;
			}
			sb.Append(L")\n{\n");
			sb.Append(L"EmitInstruction(");
			/*if (hasRMByte)
				sb.WriteString(L"true, ");
			else
				sb.WriteString(L"false, ");*/
			sb.Append(format.OpcodeLength);
			sb.Append(L", \"");
			for (int z = 0; z<format.OpcodeLength; z++)
			{
				wchar_t buf[255];
				_itow_s((unsigned char)format.Opcode[z], buf, 16);
				sb.Append(L"\\x");
				sb.Append(buf);
			}
			sb.Append(L"\", ");
			switch (format.Plus)
			{
			case vipRegister:
				sb.Append(L"GetRegisterCode(");
				if (format.Params[0] == vipREG_32 || format.Params[0] == vipREG_16 || format.Params[0] == vipREG_8)
				{
					sb.Append(L"op1.Reg");
					paramInclude[0]++;
				}
				else
				{
					sb.Append(L"op2.Reg");
					paramInclude[1]++;
				}
				sb.Append(L")");
				break;
			case vipFloat:
				if (format.Params[0] == vipST_I)
				{
					sb.Append(L"op1.Value");
					paramInclude[0]++;
				}
				else
				{
					sb.Append(L"op2.Value");
					paramInclude[1]++;
				}
				break;
			case vipNoPlus:
				sb.Append(L"0");
				break;
			default:
				break;
			}
			sb.Append(L", ");
			switch (format.Ext)
			{
				case vie0:
					sb.Append(L"0");
					break;
				case vie1:
					sb.Append(L"1");
					break;
				case vie2:
					sb.Append(L"2");
					break;
				case vie3:
					sb.Append(L"3");
					break;
				case vie4:
					sb.Append(L"4");
					break;
				case vie5:
					sb.Append(L"5");
					break;
				case vie6:
					sb.Append(L"6");
					break;
				case vie7:
					sb.Append(L"7");
					break;
				case vieRegister:
					sb.Append(L"GetRegisterCode(");
					if (format.Params[0] == vipREG_32 || format.Params[0] == vipREG_16 || format.Params[0] == vipREG_8)
					{
						sb.Append(L"op1.Reg");
						paramInclude[0]++;
					}
					else if (format.Params[1] == vipREG_32 || format.Params[1] == vipREG_16 || format.Params[1] == vipREG_8)
					{
						sb.Append(L"op2.Reg");
						paramInclude[1]++;
					}
					sb.Append(L")");
					break;
				case vieNoExt:
					if (format.Plus != vipRegister)
					{
						if (format.Params[0] == vipREG_32 || format.Params[0] == vipREG_16 || format.Params[0] == vipREG_8)
						{
							sb.Append(L"op1.Reg");
							paramInclude[0]++;
						}
						else if (format.Params[1] == vipREG_32 || format.Params[1] == vipREG_16 || format.Params[1] == vipREG_8)
						{
							sb.Append(L"op2.Reg");
							paramInclude[1]++;
						}
						else
						{
							sb.Append(L"0");
						}
					}
					else
					{
						sb.Append(L"0");
					}
					break;
			}
			sb.Append(L", ");
			if (format.Params[0] == vipRM_16 || format.Params[0] == vipRM_32 || format.Params[0] == vipRM_8 ||
				format.Params[0] == vipMF_32 || format.Params[0] == vipMF_64 || format.Params[0] == vipMI_32
				|| format.Params[0] == vipMI_16 || format.Params[0] == vipMI_64)
			{
				sb.Append(L"op1");
				paramInclude[0]++;
			}
			else if (format.Params[1] == vipRM_16 || format.Params[1] == vipRM_32 || format.Params[1] == vipRM_8||
				format.Params[1] == vipMF_32 || format.Params[1] == vipMF_64 || format.Params[1] == vipMI_32
				|| format.Params[1] == vipMI_16 || format.Params[1] == vipMI_64)
			{
				sb.Append(L"op2");
				paramInclude[1]++;
			}
			else
			{
				sb.Append(L"Operand()");
			}
			sb.Append(L");\n");
			//if (format.Imm == viiConstant)
			{
				if (format.Params[0] == vipIMM_8 || format.Params[0] == vipREL_8)
				{
					sb.Append(L"code.Add((unsigned char)op1.Value);\n");
					paramInclude[0]++;
				}
				else if (format.Params[1] == vipIMM_8 || format.Params[1] == vipREL_8)
				{
					sb.Append(L"code.Add((unsigned char)op2.Value);\n");
					paramInclude[1]++;
				}
				else if (format.Params[0] == vipIMM_16 || format.Params[0] == vipREL_16)
				{
					sb.Append(L"code.Add(((unsigned char*)&(op1.Value))[0]);\n");
					sb.Append(L"code.Add(((unsigned char*)&(op1.Value))[1]);\n");
					paramInclude[0]++;
				}
				else if (format.Params[1] == vipIMM_16 || format.Params[1] == vipREL_16)
				{
					sb.Append(L"code.Add(((unsigned char*)&(op2.Value))[0]);\n");
					sb.Append(L"code.Add(((unsigned char*)&(op2.Value))[1]);\n");
					paramInclude[1]++;
				}
				else if (format.Params[0] == vipIMM_32 || format.Params[0] == vipREL_32)
				{
					sb.Append(L"code.Add(((unsigned char*)&(op1.Value))[0]);\n");
					sb.Append(L"code.Add(((unsigned char*)&(op1.Value))[1]);\n");
					sb.Append(L"code.Add(((unsigned char*)&(op1.Value))[2]);\n");
					sb.Append(L"code.Add(((unsigned char*)&(op1.Value))[3]);\n");
					paramInclude[0]++;
				}
				else if (format.Params[1] == vipIMM_32 || format.Params[1] == vipREL_32)
				{
					sb.Append(L"code.Add(((unsigned char*)&(op2.Value))[0]);\n");
					sb.Append(L"code.Add(((unsigned char*)&(op2.Value))[1]);\n");
					sb.Append(L"code.Add(((unsigned char*)&(op2.Value))[2]);\n");
					sb.Append(L"code.Add(((unsigned char*)&(op2.Value))[3]);\n");
					paramInclude[1]++;
				}
			}
			sb.Append(L"return;\n");
			sb.Append(L"}\n");
			for (int z = 0; z<paramCount; z++)
				if (paramInclude[z] != 1)
					throw "invalid instruction generated";
		}
		sb.Append(L"throw InvalidProgramException(L\"Illegal operands.\");\n");
		sb.Append(L"}\n\n");
	}
	sb.Append(L"}\n}\n");
	FILE * f;
	_wfopen_s(&f, L"D:\\code.txt", L"wt");
	fwprintf(f, L"%s", sb.Buffer());
	fclose(f);
	_wfopen_s(&f, L"D:\\header.txt", L"wt");
	fwprintf(f, L"%s", header.Buffer());
	fclose(f);
	return 0;
}

