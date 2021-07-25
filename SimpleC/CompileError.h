#ifndef SIMPLE_C_COMPILE_ERROR_H
#define SIMPLE_C_COMPILE_ERROR_H

#include "Basic.h"

namespace SimpleC
{
	namespace Compiler
	{
		using namespace CoreLib::Basic;

		class CompileError
		{
		public:
			String Message;
			String FileName;
			int ErrorID;
			int Line, Col;

			CompileError()
			{
				ErrorID = -1;
				Line = Col = -1;
			}
			CompileError(const String & msg, const String & fileName,
						int id, int line, int col)
			{
				Message = msg;
				FileName = fileName;
				ErrorID = id;
				Line = line;
				Col = col;
			}
		};
	}
}

#endif