#ifndef _SIMPLEC_SYNTAX_PRINTER_H
#define _SIMPLEC_SYNTAX_PRINTER_H

#include "IL/CompileError.h"
#include "Syntax.h"
#include <memory>
#include <vector>

namespace SimpleC
{
	namespace Compiler
	{
		using namespace ::Compiler;
		std::unique_ptr<SyntaxVisitor> CreateSyntaxPrinter();
		std::unique_ptr<SyntaxVisitor> CreateSemanticsVisitor(std::vector<CompileError> & errors);
		std::unique_ptr<SyntaxVisitor> CreateCodeGenerator();
	}
}

#endif
