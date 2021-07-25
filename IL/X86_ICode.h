#ifndef COMPILER_X86_INSTRUCTIONS_H
#define COMPILER_X86_INSTRUCTIONS_H

#include "IntermediateCode.h"

namespace Compiler
{
	namespace Intermediate
	{
		class x86_Operation
		{
		public:
			static Operation::OperationPtr JNZ, JZ, JLE, JGE, JL, JG, JE, JNE, JA, JAE, JB, JBE;
			static Operation::OperationPtr CMP, FCOM;
		};
	}
}

#endif