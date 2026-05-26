#ifndef CORE_LIB_TEXT_IO_H
#define CORE_LIB_TEXT_IO_H

#include "Stream.h"
#include <clocale>
#include <cstring>
#include <cwchar>
#include <vector>

namespace CoreLib
{
	namespace IO
	{
		using CoreLib::Basic::_EndLine;

		//class TextReader abstract: public CoreLib::Basic::Object	
		// abstract 是ms特有关键字，无意义。
		class TextReader : public CoreLib::Basic::Object	//为什么一切都要继承自Object呢，跟Qt学的？为了方便类型转换吗？
		{
		public:
			virtual ~TextReader()
			{
				Close();
			}
			virtual void Close(){}
			virtual wchar_t Read()=0;
			virtual wchar_t Peak()=0;
			virtual int Read(wchar_t * buffer, int count)=0;
			virtual String ReadLine()=0;
			virtual String ReadToEnd()=0;
		};

		class TextWriter abstract: public CoreLib::Basic::Object
		{
		public:
			virtual ~TextWriter()
			{
				Close();
			}
			virtual void Write(const String & str)=0;
			virtual void Write(const wchar_t * str, int length=0)=0;
			virtual void Write(const char * str, int length=0)=0;
			virtual void Close(){}
			template<typename T>
			TextWriter & operator << (const T& val)
			{
				Write(val.ToString());
				return *this;
			}
			TextWriter & operator << (wchar_t value)
			{
				Write(String(value));
				return *this;
			}
			TextWriter & operator << (int value)
			{
				Write(String(value));
				return *this;
			}
			TextWriter & operator << (float value)
			{
				Write(String(value));
				return *this;
			}
			TextWriter & operator << (double value)
			{
				Write(String(value));
				return *this;
			}
			TextWriter & operator << (const char* value)
			{
				if (value != nullptr)
					Write(value, static_cast<int>(std::strlen(value)));
				return *this;
			}
			TextWriter & operator << (const wchar_t * const val)
			{
				if (val != nullptr)
					Write(val, static_cast<int>(std::wcslen(val)));
				return *this;
			}
			TextWriter & operator << (wchar_t * const val)
			{
				if (val != nullptr)
					Write(val, static_cast<int>(std::wcslen(val)));
				return *this;
			}

			TextWriter & operator << (const String & val)
			{
				Write(val);
				return *this;
			}
			TextWriter & operator << (const _EndLine &)
			{
#ifdef WINDOWS_PLATFORM
				Write(L"\r\n", 2);
#else
				Write(L"\n", 1);
#endif
				return *this;
			}
		};

		class Encoding
		{
		public:
			static Encoding * Unicode, * Ansi;
			virtual ~Encoding() = default;
			virtual std::vector<char> GetBytes(const String & str)=0;
			virtual String GetString(const char * buffer, int length)=0;
			String GetString(const std::vector<char> & buffer)
			{
				return GetString(buffer.data(), static_cast<int>(buffer.size()));
			}
		};

		class StreamWriter : public TextWriter
		{
		private:
			RefPtr<Stream> stream;
			Encoding * encoding;
		public:
			StreamWriter(const String & path, Encoding * encoding = Encoding::Unicode);
			StreamWriter(RefPtr<Stream> stream, Encoding * encoding = Encoding::Unicode);
			virtual ~StreamWriter()
			{
				Close();
			}
			virtual void Write(const String & str);
			virtual void Write(const wchar_t * str, int length=0);
			virtual void Write(const char * str, int length=0);
			virtual void Close()
			{
				if (stream)
					stream->Close();
			}
		};

		class StreamReader : public TextReader
		{
		private:
			RefPtr<Stream> stream;
			std::vector<char> buffer;
			Encoding * encoding;
			int ptr;
			char ReadBufferChar();
			void ReadBuffer();
			template<typename GetFunc>
			wchar_t GetChar(GetFunc get)
			{
				wchar_t rs = 0;
				if (encoding == Encoding::Unicode)
				{
					const auto low = static_cast<unsigned char>(get());
					const auto high = static_cast<unsigned char>(get());
					rs = static_cast<wchar_t>(low | (high << 8));
				}
				else
				{
					std::mbstate_t state{};
					char mb[MB_LEN_MAX]{};
					mb[0] = get();
					for (int length = 1; length <= MB_LEN_MAX; length++)
					{
						const auto result = std::mbrtowc(&rs, mb, static_cast<size_t>(length), &state);
						if (result != static_cast<size_t>(-2) && result != static_cast<size_t>(-1))
							return rs;
						if (length < MB_LEN_MAX)
							mb[static_cast<size_t>(length)] = get();
					}

					throw CoreLib::Basic::InvalidOperationException(L"Invalid multibyte character sequence.");
				}
				return rs;
			}
			Encoding * DetermineEncoding();
		public:
			StreamReader(const String & path);
			StreamReader(RefPtr<Stream> stream, Encoding * encoding = Encoding::Ansi);
			virtual ~StreamReader()
			{
				Close();
			}
			
			virtual wchar_t Read()
			{
				return GetChar([&]{return ReadBufferChar();});
			}
			virtual wchar_t Peak()
			{
				const int savedPtr = ptr;
				const std::vector<char> savedBuffer = buffer;
				const __int64 savedPosition = stream ? stream->GetPosition() : 0;
				try
				{
					const wchar_t value = Read();
					if (stream)
						stream->Seek(SeekOrigin::Start, savedPosition);
					buffer = savedBuffer;
					ptr = savedPtr;
					return value;
				}
				catch (...)
				{
					if (stream)
						stream->Seek(SeekOrigin::Start, savedPosition);
					buffer = savedBuffer;
					ptr = savedPtr;
					throw;
				}
			}
			virtual int Read(wchar_t * buffer, int count);
			virtual String ReadLine();
			virtual String ReadToEnd();

			virtual void Close()
			{
				if (stream)
					stream->Close();
			}
		};
	}
}

#endif
