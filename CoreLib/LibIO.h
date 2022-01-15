#ifndef CORE_LIB_IO_H	//防止重复包含头文件。
#define CORE_LIB_IO_H

#include "LibString.h"
#include "Stream.h"
#include "TextIO.h"

namespace CoreLib
{
	namespace IO
	{
		class File
		{
		public:
			static bool Exists(const CoreLib::Basic::String & fileName);
			static CoreLib::Basic::String ReadAllText(const CoreLib::Basic::String & fileName);
		};

		class Path
		{
		public:
			static String TruncateExt(const String & path);
			static String ReplaceExt(const String & path, const wchar_t * newExt);
			static String GetFileName(const String & path);
			static String GetFileExt(const String & path);
			static String GetDirectoryName(const String & path);
			static String Combine(const String & path1, const String & path2);
			static String Combine(const String & path1, const String & path2, const String & path3);
		};
	}
}

#endif