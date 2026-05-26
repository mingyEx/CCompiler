#ifndef CORE_LIB_STREAM_H
#define CORE_LIB_STREAM_H

#include <climits>
#include <fstream>
#include <memory>
#include <vector>

#include "Basic.h"

namespace CoreLib
{
	namespace IO
	{
		using CoreLib::Basic::Exception;
		using CoreLib::Basic::String;
		using CoreLib::Basic::RefPtr;

		class IOException : public Exception
		{
		public:
			IOException()
			{}
			IOException(const String & message)
				: CoreLib::Basic::Exception(message)
			{}
		};

		class EndOfStreamException : public IOException
		{
		public:
			EndOfStreamException()
			{}
			EndOfStreamException(const String & message)
				: IOException(message)
			{
			}
		};

		enum class SeekOrigin
		{
			Start, End, Current
		};

		class Stream : public CoreLib::Basic::Object
		{
		public:
			virtual ~Stream() = default;
			virtual __int64 GetPosition()=0;
			virtual void Seek(SeekOrigin origin, __int64 offset)=0;
			virtual int Read(void * buffer, int length)=0;
			virtual int Write(const void * buffer, int length)=0;
			virtual bool CanRead()=0;
			virtual bool CanWrite()=0;
			virtual void Close()=0;
		};

		class BinaryReader
		{
		private:
			RefPtr<Stream> stream;
			void ReadExact(void * buffer, int byteCount)
			{
				char * output = static_cast<char *>(buffer);
				int totalBytesRead = 0;
				while (totalBytesRead < byteCount)
				{
					const int bytesRead = stream->Read(output + totalBytesRead, byteCount - totalBytesRead);
					if (bytesRead <= 0)
						throw EndOfStreamException(L"End of stream reached before the requested data was read.");
					totalBytesRead += bytesRead;
				}
			}
		public:
			BinaryReader(RefPtr<Stream> stream)
			{
				this->stream = stream;
			}
			Stream * GetStream()
			{
				return stream.Ptr();
			}
			const Stream * GetStream() const
			{
				return stream.Ptr();
			}
			template<typename T>
			void Read(T * buffer, int count)
			{
				if (count < 0)
					throw CoreLib::Basic::ArgumentException(L"Read count cannot be negative.");
				if (count == 0)
					return;
				if (buffer == nullptr)
					throw CoreLib::Basic::ArgumentException(L"Read buffer cannot be null.");
				if (count > (INT_MAX / static_cast<int>(sizeof(T))))
					throw CoreLib::Basic::ArgumentException(L"Read count is too large.");
				const auto byteCount = static_cast<int>(sizeof(T) * static_cast<size_t>(count));
				ReadExact(buffer, byteCount);
			}
			int ReadInt32()
			{
				int rs;
				ReadExact(&rs, sizeof(int));
				return rs;
			}
			short ReadInt16()
			{
				short rs;
				ReadExact(&rs, sizeof(short));
				return rs;
			}
			__int64 ReadInt64()
			{
				__int64 rs;
				ReadExact(&rs, sizeof(__int64));
				return rs;
			}
			float ReadFloat()
			{
				float rs;
				ReadExact(&rs, sizeof(float));
				return rs;
			}
			double ReadDouble()
			{
				double rs;
				ReadExact(&rs, sizeof(double));
				return rs;
			}
			char ReadChar()
			{
				char rs;
				ReadExact(&rs, sizeof(char));
				return rs;
			}
			String ReadString()
			{
				int len = ReadInt32();
				if (len < 0)
					throw IOException(L"Invalid string length.");
				if (len == 0)
					return String();
				if (len > (INT_MAX / static_cast<int>(sizeof(wchar_t))))
					throw IOException(L"String byte length is too large.");
				std::vector<wchar_t> buffer(static_cast<size_t>(len) + 1);
				ReadExact(buffer.data(), static_cast<int>(sizeof(wchar_t) * static_cast<size_t>(len)));
				buffer[static_cast<size_t>(len)] = 0;
				return String(buffer.data());
			}
		};

		class BinaryWriter
		{
		private:
			RefPtr<Stream> stream;
		public:
			BinaryWriter(RefPtr<Stream> stream)
			{
				this->stream = stream;
			}
			Stream * GetStream()
			{
				return stream.Ptr();
			}
			const Stream * GetStream() const
			{
				return stream.Ptr();
			}
			template<typename T>
			void Write(const T& val)
			{
				stream->Write(&val, sizeof(T));
			}
			template<typename T>
			void Write(const T * buffer, int count)
			{
				if (count < 0)
					throw CoreLib::Basic::ArgumentException(L"Write count cannot be negative.");
				if (count == 0)
					return;
				if (buffer == nullptr)
					throw CoreLib::Basic::ArgumentException(L"Write buffer cannot be null.");
				if (count > (INT_MAX / static_cast<int>(sizeof(T))))
					throw CoreLib::Basic::ArgumentException(L"Write count is too large.");
				const auto byteCount = static_cast<int>(sizeof(T) * static_cast<size_t>(count));
				stream->Write(buffer, byteCount);
			}
			void Write(const String & str)
			{
				Write(str.Length());
				Write(str.Buffer(), str.Length());
			}
			void Close()
			{
				stream->Close();
			}
		};

		enum class FileMode
		{
			Create, Open, CreateNew, Append
		};

		enum class FileAccess
		{
			Read = 1, Write = 2, ReadWrite = 3
		};

		enum class FileShare
		{
			None, ReadOnly, WriteOnly, ReadWrite
		};

		class FileStream : public Stream
		{
		private:
			std::unique_ptr<std::fstream> handle;
			FileAccess fileAccess;
			void Init(const CoreLib::Basic::String & fileName, FileMode fileMode, FileAccess access, FileShare share);
		public:
			FileStream(const CoreLib::Basic::String & fileName, FileMode fileMode = FileMode::Open);
			FileStream(const CoreLib::Basic::String & fileName, FileMode fileMode, FileAccess access, FileShare share);
			FileStream(const FileStream &) = delete;
			FileStream & operator=(const FileStream &) = delete;
			FileStream(FileStream &&) = delete;
			FileStream & operator=(FileStream &&) = delete;
			~FileStream();
		public:
			virtual __int64 GetPosition();
			virtual void Seek(SeekOrigin origin, __int64 offset);
			virtual int Read(void * buffer, int length);
			virtual int Write(const void * buffer, int length);
			virtual bool CanRead();
			virtual bool CanWrite();
			virtual void Close();
		};
	}
}

#endif
