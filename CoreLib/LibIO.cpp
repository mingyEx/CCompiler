#include "LibIO.h"
#include "Exception.h"
#include "TextIO.h"
#include <filesystem>

namespace CoreLib
{
	namespace IO
	{
		using namespace CoreLib::Basic;

		bool File::Exists(const String & fileName)
		{
			if (fileName.Length() == 0)
				return false;
			return std::filesystem::exists(std::filesystem::path(fileName.Buffer()));
		}

		String Path::TruncateExt(const String & path)
		{
			if (path.Length() == 0)
				return String();
			auto nativePath = std::filesystem::path(path.Buffer());
			nativePath.replace_extension();
			return String(nativePath.wstring().c_str());
		}
		String Path::ReplaceExt(const String & path, const wchar_t * newExt)
		{
			if (path.Length() == 0)
				return String();
			auto nativePath = std::filesystem::path(path.Buffer());
			if (newExt == nullptr || newExt[0] == 0)
				nativePath.replace_extension();
			else
				nativePath.replace_extension(newExt[0] == L'.' ? std::wstring(newExt) : std::wstring(L".") + newExt);
			return String(nativePath.wstring().c_str());
		}
		String Path::GetFileName(const String & path)
		{
			if (path.Length() == 0)
				return String();
			auto nativePath = std::filesystem::path(path.Buffer());
			return String(nativePath.filename().wstring().c_str());
		}
		String Path::GetFileExt(const String & path)
		{
			if (path.Length() == 0)
				return String();
			auto extension = std::filesystem::path(path.Buffer()).extension().wstring();
			if (!extension.empty() && extension[0] == L'.')
				extension.erase(0, 1);
			return String(extension.c_str());
		}
		String Path::GetDirectoryName(const String & path)
		{
			if (path.Length() == 0)
				return String();
			auto nativePath = std::filesystem::path(path.Buffer());
			return String(nativePath.parent_path().wstring().c_str());
		}
		String Path::Combine(const String & path1, const String & path2)
		{
			if (path1.Length() == 0)
				return path2;
			if (path2.Length() == 0)
				return path1;
			auto combined = std::filesystem::path(path1.Buffer()) / path2.Buffer();
			return String(combined.wstring().c_str());
		}
		String Path::Combine(const String & path1, const String & path2, const String & path3)
		{
			if (path1.Length() == 0)
				return Combine(path2, path3);
			if (path2.Length() == 0)
				return Combine(path1, path3);
			if (path3.Length() == 0)
				return Combine(path1, path2);
			auto combined = std::filesystem::path(path1.Buffer()) / path2.Buffer() / path3.Buffer();
			return String(combined.wstring().c_str());
		}

		CoreLib::Basic::String File::ReadAllText(const CoreLib::Basic::String & fileName)
		{
			if (fileName.Length() == 0)
				throw IOException(L"Failed to open file: path is empty.");
			StreamReader reader(fileName);
			return reader.ReadToEnd();
		}
	}
}
