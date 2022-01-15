#include "LibIO.h"
#include "Exception.h"

#ifdef WINDOWS_PLATFORM
#include <Windows.h>
#include "Shlwapi.h"
#endif

namespace CoreLib
{
	namespace IO
	{
		using namespace CoreLib::Basic;

		bool File::Exists(const String & fileName)
		{
#ifdef WINDOWS_PLATFORM
			return PathFileExistsW(fileName.Buffer()) != 0;
#else
			throw NotImplementedException(L"FileExists not implemented for current platform.");
#endif
		}

		String Path::TruncateExt(const String & path)
		{
			int dotPos = path.LastIndexOf(L'.');
			if (dotPos != -1)
				return path.SubString(0, dotPos);
			else
				return path;
		}
		String Path::ReplaceExt(const String & path, const wchar_t * newExt)
		{
			StringBuilder sb(path.Length()+10);
			int dotPos = path.LastIndexOf(L'.');
			if (dotPos == -1)
				dotPos = path.Length();
			sb.Append(path.Buffer(), dotPos);
			sb.Append(L'.');
			sb.Append(newExt);
			return sb.ProduceString();
		}
		String Path::GetFileName(const String & path)
		{
			int pos = path.LastIndexOf(L'/');
			pos = Math::Max(path.LastIndexOf(L'\\'), pos) + 1;
			return path.SubString(pos, path.Length()-pos);
		}
		String Path::GetFileExt(const String & path)
		{
			int dotPos = path.LastIndexOf(L'.');
			if (dotPos != -1)
				return path.SubString(dotPos+1, path.Length()-dotPos-1);
			else
				return L"";
		}
		String Path::GetDirectoryName(const String & path)
		{
			int pos = path.LastIndexOf(L'/');
			pos = Math::Max(path.LastIndexOf(L'\\'), pos);
			return path.SubString(0, pos);
		}
		String Path::Combine(const String & path1, const String & path2)
		{
			StringBuilder sb(path1.Length()+path2.Length()+2);
			sb.Append(path1);
			if (!path1.EndsWith(L'\\') && !path1.EndsWith(L'/'))
				sb.Append(L'\\');
			sb.Append(path2);
			return sb.ProduceString();
		}
		String Path::Combine(const String & path1, const String & path2, const String & path3)
		{
			StringBuilder sb(path1.Length()+path2.Length()+path3.Length()+3);
			sb.Append(path1);
			if (!path1.EndsWith(L'\\') && !path1.EndsWith(L'/'))
				sb.Append(L'\\');
			sb.Append(path2);
			if (!path2.EndsWith(L'\\') && !path2.EndsWith(L'/'))
				sb.Append(L'\\');
			sb.Append(path3);
			return sb.ProduceString();
		}

		CoreLib::Basic::String File::ReadAllText(const CoreLib::Basic::String & fileName)
		{
			StreamReader reader(new FileStream(fileName, FileMode::Open, FileAccess::Read, FileShare::ReadWrite));
			return reader.ReadToEnd();
		}
	}
}