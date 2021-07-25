#ifndef X86_CODE_GEN_H
#define X86_CODE_GEN_H

#include "IntermediateCode.h"
#include "Assembly_x86.h"

namespace Compiler
{
	namespace Intermediate
	{
		class X86CodeGenerator
		{
		public:
			virtual x86::Program_x86 GenerateCode(Program * program) = 0;
		};

		X86CodeGenerator * CreateX86CodeGenerator();
	}
}

#endif