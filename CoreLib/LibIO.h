#ifndef CORE_LIB_IO_H	//防止重复包含头文件。
#define CORE_LIB_IO_H

#include "LibString.h"

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
			static CoreLib::Basic::String TruncateExt(const CoreLib::Basic::String & path);
			static CoreLib::Basic::String ReplaceExt(const CoreLib::Basic::String & path, const wchar_t * newExt);
			static CoreLib::Basic::String GetFileName(const CoreLib::Basic::String & path);
			static CoreLib::Basic::String GetFileExt(const CoreLib::Basic::String & path);
			static CoreLib::Basic::String GetDirectoryName(const CoreLib::Basic::String & path);
			static CoreLib::Basic::String Combine(const CoreLib::Basic::String & path1, const CoreLib::Basic::String & path2);
			static CoreLib::Basic::String Combine(const CoreLib::Basic::String & path1, const CoreLib::Basic::String & path2, const CoreLib::Basic::String & path3);
		};
	}
}

#endif
