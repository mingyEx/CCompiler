#include "TextIO.h"
#include <ctype.h>
#ifdef WINDOWS_PLATFORM
#include <Windows.h>
#endif
namespace CoreLib
{
	namespace IO
	{
		using namespace CoreLib::Basic;

		class UnicodeEncoding : public Encoding
		{
		public:
			virtual List<char> GetBytes(const String & str)
			{
				List<char> rs;
				rs.Reserve(str.Length());
				rs.AddRange((char*)str.Buffer(), str.Length()*2);
				return rs;
			}

			virtual String GetString(char * buffer, int length)
			{
#ifdef WINDOWS_PLATFORM
				int unicodeFlag;
				int startPos = 0;
				IsTextUnicode(buffer, length, &unicodeFlag);
				if (unicodeFlag & IS_TEXT_UNICODE_UNICODE_MASK)
				{
					if (unicodeFlag & IS_TEXT_UNICODE_SIGNATURE)
					{
						startPos += 2;
					}
					length = wcsnlen_s((wchar_t*)buffer+startPos, length-startPos);
					wchar_t * rbuffer = new wchar_t[length+1];
					memcpy(rbuffer, buffer, length*sizeof(wchar_t));
					return String::FromBuffer(rbuffer, length);
				}
				else if (unicodeFlag & IS_TEXT_UNICODE_REVERSE_MASK)
				{
					if (unicodeFlag & IS_TEXT_UNICODE_REVERSE_SIGNATURE)
					{
						startPos += 2;
					}
					length = wcsnlen_s((wchar_t*)buffer+startPos, length-startPos);
					wchar_t * rbuffer = new wchar_t[length+1];
					for (int i = 0; i<length; i++)
					{
						((char*)rbuffer)[i*2] = buffer[i*2+1];
						((char*)rbuffer)[i*2+1] = buffer[i*2];
					}
					rbuffer[length] = 0;
					return String::FromBuffer(rbuffer, length);
				}
				else
				{
					throw ArgumentException(L"Invalid unicode text format.");
				}
#else
				throw NotImplementedException(L"UnicodeEncoding::GetString() not implemented for current platform.");
#endif
			}
		};

		class AnsiEncoding : public Encoding
		{
		private:
			static char * WideCharToAnsi(wchar_t * buffer, int length)
			{
				size_t requiredBufferSize;
				requiredBufferSize = WideCharToMultiByte(CP_OEMCP, NULL, buffer, length, 0, 0, NULL, NULL)+1;
				if (requiredBufferSize)
				{
					char * multiByteBuffer = new char[requiredBufferSize];
					WideCharToMultiByte(CP_OEMCP, NULL, buffer, length, multiByteBuffer, requiredBufferSize, NULL, NULL);
					multiByteBuffer[requiredBufferSize-1] = 0;
					return multiByteBuffer;
				}
				else
					return 0;
			}
		public:
			virtual List<char> GetBytes(const String & str)
			{
				List<char> rs;
				String cpy = str;
				int len;
				char * buffer = cpy.ToMultiByteString(&len);
				rs.AddRange(buffer, len);
				return rs;
			}

			virtual String GetString(char * buffer, int length)
			{
#ifdef WINDOWS_PLATFORM
				// regard as ansi
				int rlength = MultiByteToWideChar(CP_ACP, 0, buffer, length, NULL, 0)-1;
				if (length < 0) length = 0;
				if (length != 0)
				{
					wchar_t * rbuffer = new wchar_t[length+1];
					MultiByteToWideChar(CP_ACP, NULL, buffer, length, rbuffer, length+1);
					return String::FromBuffer(rbuffer, length);
				}
				else
					return String();
#else
				throw NotImplementedException(L"UnicodeEncoding::GetString() not implemented for current platform.");
#endif
			}
		};

		UnicodeEncoding __unicodeEncoding;
		AnsiEncoding __ansiEncoding;

		Encoding * Encoding::Unicode = &__unicodeEncoding;
		Encoding * Encoding::Ansi = &__ansiEncoding;

		StreamWriter::StreamWriter(const String & path, Encoding * encoding)
		{
			this->stream = new FileStream(path, FileMode::Create);
			this->encoding = encoding;
		}
		StreamWriter::StreamWriter(RefPtr<Stream> stream, Encoding * encoding)
		{
			this->stream = stream;
			this->encoding = encoding;
		}
		void StreamWriter::Write(const String & str)
		{
			if (encoding == Encoding::Unicode)
			{
				stream->Write(str.Buffer(), str.Length()*sizeof(wchar_t));
			}
			else
			{
				auto bytes = encoding->GetBytes(String(str));
				stream->Write(bytes.Buffer(), bytes.Count());
			}
		}
		void StreamWriter::Write(const wchar_t * str, int length)
		{
			if (encoding == Encoding::Unicode)
			{
				stream->Write(str, length*sizeof(wchar_t));
			}
			else
			{
				auto bytes = encoding->GetBytes(String(str));
				stream->Write(bytes.Buffer(), bytes.Count());
			}
		}
		void StreamWriter::Write(const char * str, int length)
		{
			if (encoding == Encoding::Ansi)
			{
				stream->Write(str, length*sizeof(char));
			}
			else
			{
				String cpy(str);
				stream->Write(cpy.Buffer(), sizeof(wchar_t) * cpy.Length());
			}
		}

		StreamReader::StreamReader(const String & path)
		{
			stream = new FileStream(path, FileMode::Open);
			ReadBuffer();
			encoding = DetermineEncoding();
			if (encoding == 0)
				encoding = Encoding::Ansi;
		}
		StreamReader::StreamReader(RefPtr<Stream> stream, Encoding * encoding)
		{
			this->stream = stream;
			this->encoding = encoding;
			ReadBuffer();
			encoding = DetermineEncoding();
			if (this->encoding == 0)
				this->encoding = encoding;
		}

		Encoding * StreamReader::DetermineEncoding()
		{
			int flag = 0;
			int rs = IsTextUnicode(buffer.Buffer(), buffer.Count(), &flag);
			if (flag & (IS_TEXT_UNICODE_SIGNATURE | IS_TEXT_UNICODE_REVERSE_SIGNATURE))
				return Encoding::Unicode;
			else if (rs)
				return Encoding::Ansi;
			else
				return 0;
		}
		
		void StreamReader::ReadBuffer()
		{
			buffer.SetSize(4096);
			int len = stream->Read(buffer.Buffer(), buffer.Count());
			buffer.SetSize(len);
			ptr = 0;
		}

		char StreamReader::ReadBufferChar()
		{
			if (ptr<buffer.Count())
			{
				return buffer[ptr++];
			}
			ReadBuffer();
			if (ptr<buffer.Count())
			{
				return buffer[ptr++];
			}
			return 0;
		}

		char StreamReader::PeakBufferChar()
		{
			if (ptr<buffer.Count())
			{
				return buffer[ptr];
			}
			try
			{
				ReadBuffer();
			}
			catch (EndOfStreamException)
			{
				buffer.Clear();
				ptr = 0;
			}
			if (ptr<buffer.Count())
			{
				return buffer[ptr];
			}
			return 0;
		}
		int StreamReader::Read(wchar_t * buffer, int length)
		{
			int i = 0;
			for (i = 0; i<length; i++)
			{
				try
				{
					auto ch = Read();
					if (ch == L'\r')
					{
						if (Peak() == L'\n')
							Read();
						break;
					}
					else if (ch == L'\n')
					{
						break;
					}
					buffer[i] = ch;
				}
				catch (EndOfStreamException)
				{
					break;
				}
			}
			return i;
		}
		String StreamReader::ReadLine()
		{
			StringBuilder sb(256);
			bool ignoreLineBreak = false;
			while (true)
			{
				try
				{
					auto ch = Read();
					if (ch == L'\r')
					{
						if (Peak() == L'\n')
							Read();
						break;
					}
					else if (ch == L'\n')
					{
						break;
					}
					sb.Append(ch);
				}
				catch (EndOfStreamException)
				{
					break;
				}
			}
			return sb.ProduceString();
		}
		String StreamReader::ReadToEnd()
		{
			StringBuilder sb(16384);
			bool ignoreLineBreak = false;
			while (true)
			{
				try
				{
					auto ch = Read();
					if (ch == L'\r')
					{
						sb.Append(L'\n');
						if (Peak() == L'\n')
							Read();
					}
					else
						sb.Append(ch);
				}
				catch (EndOfStreamException)
				{
					break;
				}
			}
			return sb.ProduceString();
		}
	}
}