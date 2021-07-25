#include "X86_ICode.h"

namespace Compiler
{
	namespace Intermediate
	{
		Operation OpJNZ(L"JNZ");
		Operation OpJZ(L"JZ");
		Operation OpJLE(L"JLE");
		Operation OpJGE(L"JGE");
		Operation OpJL(L"JL");
		Operation OpJG(L"JG");
		Operation OpJE(L"JE");
		Operation OpJNE(L"JNE");
		Operation OpJA(L"JA");
		Operation OpJAE(L"JAE");
		Operation OpJB(L"JB");
		Operation OpJBE(L"JBE");
		Operation OpFCOM(L"FCOMP");
		Operation OpCMP(L"CMP");
		Operation::OperationPtr x86_Operation::JNZ = &OpJNZ;
		Operation::OperationPtr x86_Operation::JZ = &OpJZ;
		Operation::OperationPtr x86_Operation::JLE = &OpJLE;
		Operation::OperationPtr x86_Operation::JGE = &OpJGE;
		Operation::OperationPtr x86_Operation::JL = &OpJL;
		Operation::OperationPtr x86_Operation::JG = &OpJG;
		Operation::OperationPtr x86_Operation::JE = &OpJE;
		Operation::OperationPtr x86_Operation::JNE = &OpJNE;
		Operation::OperationPtr x86_Operation::JA = &OpJA;
		Operation::OperationPtr x86_Operation::JAE = &OpJAE;
		Operation::OperationPtr x86_Operation::JB = &OpJB;
		Operation::OperationPtr x86_Operation::JBE = &OpJBE;
		Operation::OperationPtr x86_Operation::FCOM = &OpFCOM;
		Operation::OperationPtr x86_Operation::CMP = &OpCMP;
	}
}