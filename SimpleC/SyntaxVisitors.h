#ifndef _SIMPLEC_SYNTAX_PRINTER_H
#define _SIMPLEC_SYNTAX_PRINTER_H

#include "IL/CompileError.h"
#include "Syntax.h"

namespace SimpleC
{
	namespace Compiler
	{
		using namespace ::Compiler;
		SyntaxVisitor * CreateSyntaxPrinter();
		SyntaxVisitor * CreateSemanticsVisitor(List<CompileError> & errors);
		SyntaxVisitor * CreateCodeGenerator();
	}
}

#endif