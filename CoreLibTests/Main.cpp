#include "corelib_regression_tests.h"

#include <cwchar>
#include <iostream>

int wmain(int argc, wchar_t* argv[])
{
	if (argc > 2)
	{
		std::wcerr << L"Usage: CoreLibTests [--corelib-self-test]" << std::endl;
		return 1;
	}

	if (argc == 2 && std::wcscmp(argv[1], L"--corelib-self-test") != 0)
	{
		std::wcerr << L"Usage: CoreLibTests [--corelib-self-test]" << std::endl;
		return 1;
	}

	return SimpleC::Tests::RunCoreLibRegressionTests();
}
