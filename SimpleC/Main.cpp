#include "compiler_pipeline.h"

int wmain(int argc, wchar_t* argv[])
{
	return SimpleC::Compiler::RunCompilerFromCommandLine(argc, argv);
}
