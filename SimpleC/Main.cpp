#include "compiler_pipeline.h"
#include "corelib_regression_tests.h"

int wmain(int argc, wchar_t* argv[])
{
	if (argc >= 2 && wcscmp(argv[1], L"--corelib-self-test") == 0)
		return SimpleC::Tests::RunCoreLibRegressionTests();
	return SimpleC::Compiler::RunCompilerFromCommandLine(argc, argv);
}
