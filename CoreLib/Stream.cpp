#include "Stream.h"
#include <filesystem>
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
		void FileStream::Init(const CoreLib::Basic::String & inputFileName, FileMode fileMode, FileAccess access, FileShare share)
		{
			if (inputFileName.Length() == 0)
				throw IOException(L"Cannot open file: path is empty.");

			std::ios::openmode mode = std::ios::binary;
			switch (fileMode)
			{
			case CoreLib::IO::FileMode::Create:
				if (access == FileAccess::Read)
					throw ArgumentException(L"Read-only access is incompatible with Create mode.");
				else if (access == FileAccess::ReadWrite)
				{
					mode |= std::ios::in | std::ios::out | std::ios::trunc;
					this->fileAccess = FileAccess::ReadWrite;
				}
				else
				{
					mode |= std::ios::out | std::ios::trunc;
					this->fileAccess = FileAccess::Write;
				}
				break;
			case CoreLib::IO::FileMode::Open:
				if (access == FileAccess::Read)
				{
					mode |= std::ios::in;
					this->fileAccess = FileAccess::Read;
				}
				else if (access == FileAccess::ReadWrite)
				{
					mode |= std::ios::in | std::ios::out;
					this->fileAccess = FileAccess::ReadWrite;
				}
				else
				{
					mode |= std::ios::out;
					this->fileAccess = FileAccess::Write;
				}
				break;
			case CoreLib::IO::FileMode::CreateNew:
				if (File::Exists(inputFileName))
				{
					throw IOException(L"Failed opening '" + inputFileName + L"', file already exists.");
				}
				if (access == FileAccess::Read)
					throw ArgumentException(L"Read-only access is incompatible with Create mode.");
				else if (access == FileAccess::ReadWrite)
				{
					mode |= std::ios::in | std::ios::out | std::ios::trunc;
					this->fileAccess = FileAccess::ReadWrite;
				}
				else
				{
					mode |= std::ios::out | std::ios::trunc;
					this->fileAccess = FileAccess::Write;
				}
				break;
			case CoreLib::IO::FileMode::Append:
				if (access == FileAccess::Read)
					throw ArgumentException(L"Read-only access is incompatible with Append mode.");
				else if (access == FileAccess::ReadWrite)
				{
					mode |= std::ios::in | std::ios::out | std::ios::app;
					this->fileAccess = FileAccess::ReadWrite;
				}
				else
				{
					mode |= std::ios::out | std::ios::app;
					this->fileAccess = FileAccess::Write;
				}
				break;
			default:
				throw ArgumentException(L"Invalid file mode.");
			}
			if (share != FileShare::None)
			{
				// std::fstream does not expose portable share flags. Keep behavior explicit.
				throw NotSupportedException(L"Only FileShare::None is currently supported.");
			}

			handle = std::make_unique<std::fstream>(std::filesystem::path(inputFileName.Buffer()), mode);
			if (!handle->is_open())
			{
				handle.reset();
				throw IOException(L"Cannot open file '" + inputFileName + L"'");
			}
		}
		FileStream::~FileStream()
		{
			Close();
		}
		__int64 FileStream::GetPosition()
		{
			if (!handle)
				throw IOException(L"FileStream is closed.");

			const auto originalState = handle->rdstate();
			handle->clear();
			const auto readPos = handle->tellg();
			if (readPos != static_cast<std::streampos>(-1))
			{
				handle->clear();
				handle->setstate(originalState);
				return static_cast<__int64>(readPos);
			}

			const auto writePos = handle->tellp();
			if (writePos != static_cast<std::streampos>(-1))
			{
				handle->clear();
				handle->setstate(originalState);
				return static_cast<__int64>(writePos);
			}

			handle->clear();
			handle->setstate(originalState);

			throw IOException(L"Failed to get file position.");
		}
		void FileStream::Seek(SeekOrigin origin, __int64 offset)
		{
			if (!handle)
				throw IOException(L"FileStream is closed.");

			std::ios::seekdir seekDir;
			switch (origin)
			{
			case CoreLib::IO::SeekOrigin::Start:
				seekDir = std::ios::beg;
				break;
			case CoreLib::IO::SeekOrigin::End:
				seekDir = std::ios::end;
				break;
			case CoreLib::IO::SeekOrigin::Current:
				seekDir = std::ios::cur;
				break;
			default:
				throw NotSupportedException(L"Unsupported seek origin.");
			}

			handle->clear();
			if (CanRead())
				handle->seekg(offset, seekDir);
			if (CanWrite())
				handle->seekp(offset, seekDir);
			if (handle->fail())
				throw IOException(L"FileStream seek failed.");
		}
		int FileStream::Read(void * buffer, int length)
		{
			if (!handle)
				throw IOException(L"FileStream is closed.");
			if (!CanRead())
				throw IOException(L"FileStream does not support reading.");
			if (length < 0)
				throw ArgumentException(L"Read length cannot be negative.");
			if (length == 0)
				return 0;
			if (buffer == nullptr)
				throw ArgumentException(L"Read buffer cannot be null.");

			handle->read(reinterpret_cast<char *>(buffer), length);
			const int bytes = static_cast<int>(handle->gcount());
			if (bytes == 0)
			{
				if (handle->eof())
					throw EndOfStreamException(L"End of stream reached when reading.");
				else
					throw IOException(L"FileStream read failed.");
			}
			return bytes;
		}
		int FileStream::Write(const void * buffer, int length)
		{
			if (!handle)
				throw IOException(L"FileStream is closed.");
			if (!CanWrite())
				throw IOException(L"FileStream does not support writing.");
			if (length < 0)
				throw ArgumentException(L"Write length cannot be negative.");
			if (length == 0)
				return 0;
			if (buffer == nullptr)
				throw ArgumentException(L"Write buffer cannot be null.");

			handle->write(reinterpret_cast<const char *>(buffer), length);
			if (handle->fail())
				throw IOException(L"FileStream write failed.");

			return length;
		}
		bool FileStream::CanRead()
		{
			return (static_cast<int>(fileAccess) & static_cast<int>(FileAccess::Read)) != 0;
		}
		bool FileStream::CanWrite()
		{
			return (static_cast<int>(fileAccess) & static_cast<int>(FileAccess::Write)) != 0;
		}
		void FileStream::Close()
		{
			if (handle)
			{
				handle->close();
				handle.reset();
			}
		}
	}
}
