#include "Stream.h"
#include <share.h>
#include "LibIO.h"

namespace CoreLib
{
	namespace IO
	{

		using namespace CoreLib::Basic;
		FileStream::FileStream(const CoreLib::Basic::String & fileName, FileMode fileMode)
		{
			Init(fileName, fileMode, fileMode==FileMode::Open?FileAccess::Read:FileAccess::Write, FileShare::None);
		}
		FileStream::FileStream(const CoreLib::Basic::String & fileName, FileMode fileMode, FileAccess access, FileShare share)
		{
			Init(fileName, fileMode, access, share);
		}
		void FileStream::Init(const CoreLib::Basic::String & fileName, FileMode fileMode, FileAccess access, FileShare share)
		{
			wchar_t * mode;
			switch (fileMode)
			{
			case CoreLib::IO::FileMode::Create:
				if (access == FileAccess::Read)
					throw ArgumentException(L"Read-only access is incompatible with Create mode.");
				else if (access == FileAccess::ReadWrite)
				{
					mode = L"w+b";
					this->fileAccess = FileAccess::ReadWrite;
				}
				else
				{
					mode = L"wb";
					this->fileAccess = FileAccess::Write;
				}
				break;
			case CoreLib::IO::FileMode::Open:
				if (access == FileAccess::Read)
				{
					mode = L"rb";
					this->fileAccess = FileAccess::Read;
				}
				else if (access == FileAccess::ReadWrite)
				{
					mode = L"r+b";
					this->fileAccess = FileAccess::ReadWrite;
				}
				else
				{
					mode = L"wb";
					this->fileAccess = FileAccess::Write;
				}
				break;
			case CoreLib::IO::FileMode::CreateNew:
				if (File::Exists(fileName))
				{
					throw IOException(L"Failed openning '" + fileName + L"', file alread exists.");
				}
				if (access == FileAccess::Read)
					throw ArgumentException(L"Read-only access is incompatible with Create mode.");
				else if (access == FileAccess::ReadWrite)
				{
					mode = L"w+b";
					this->fileAccess = FileAccess::ReadWrite;
				}
				else
				{
					mode = L"wb";
					this->fileAccess = FileAccess::Write;
				}
				break;
			case CoreLib::IO::FileMode::Append:
				if (access == FileAccess::Read)
					throw ArgumentException(L"Read-only access is incompatible with Append mode.");
				else if (access == FileAccess::ReadWrite)
				{
					mode = L"a+b";
					this->fileAccess = FileAccess::ReadWrite;
				}
				else
				{
					mode = L"ab";
					this->fileAccess = FileAccess::Write;
				}
				break;
			default:
				break;
			}
			int shFlag;
			switch (share)
			{
			case CoreLib::IO::FileShare::None:
				shFlag = _SH_DENYRW;
				break;
			case CoreLib::IO::FileShare::ReadOnly:
				shFlag = _SH_DENYWR;
				break;
			case CoreLib::IO::FileShare::WriteOnly:
				shFlag = _SH_DENYRD;
				break;
			case CoreLib::IO::FileShare::ReadWrite:
				shFlag = _SH_DENYNO;
				break;
			default:
				throw ArgumentException(L"Invalid file share mode.");
				break;
			}
			handle = _wfsopen(fileName.Buffer(), mode, shFlag);		
			//https://www.cnblogs.com/cnxkey/articles/8301556.html
			if (!handle)
			{
				throw IOException(L"Cannot open file '" + fileName + L"'");
			}
		}
		FileStream::~FileStream()
		{
			Close();
		}
		__int64 FileStream::GetPosition()
		{
			fpos_t pos;
			fgetpos(handle, &pos);
			return pos;
		}
		void FileStream::Seek(SeekOrigin origin, __int64 offset)
		{
			int _origin;
			switch (origin)
			{
			case CoreLib::IO::SeekOrigin::Start:
				_origin = SEEK_SET;
				break;
			case CoreLib::IO::SeekOrigin::End:
				_origin = SEEK_END;
				break;
			case CoreLib::IO::SeekOrigin::Current:
				_origin = SEEK_CUR;
				break;
			default:
				throw NotSupportedException(L"Unsupported seek origin.");
				break;
			}
			int rs = _fseeki64(handle, offset, _origin);
			if (rs != 0)
			{
				throw IOException(L"FileStream seek failed.");
			}
		}
		int FileStream::Read(void * buffer, int length)
		{
			int bytes = fread_s(buffer, length, 1, length, handle);
			if (bytes == 0)
			{
				if (feof(handle))
					throw EndOfStreamException(L"End of stream reached when reading.");
				else
					throw IOException(L"FileStream read failed.");
			}
			return bytes;
		}
		int FileStream::Write(const void * buffer, int length)
		{
			int bytes = fwrite(buffer, 1, length, handle);
			if (bytes < length)
			{
				throw IOException(L"FileStream write failed.");
			}
			return bytes;
		}
		bool FileStream::CanRead()
		{
			return ((int)fileAccess & (int)FileAccess::Read) != 0;
		}
		bool FileStream::CanWrite()
		{
			return ((int)fileAccess & (int)FileAccess::Write) != 0;
		}
		void FileStream::Close()
		{
			if (handle)
			{
				fclose(handle);
				handle = 0;
			}
		}
	}
}