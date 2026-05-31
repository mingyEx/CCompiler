#include "TextIO.h"
#include <clocale>
#include <cstring>
#include <cwchar>
#include <vector>
namespace CoreLib
{
	namespace
	{
		const bool text_io_locale_initialized = []()
		{
			std::setlocale(LC_ALL, "");
			return true;
		}();

		std::vector<char> convert_wide_to_multibyte(const wchar_t * source)
		{
			if (!source || source[0] == 0)
				return {};

			std::vector<char> result;
#ifdef _MSC_VER
			size_t converted_length = 0;
			if (wcsrtombs_s(&converted_length, nullptr, 0, &source, 0, nullptr) != 0 || converted_length == 0)
				throw CoreLib::Basic::InvalidOperationException(L"Failed to convert wide string to multibyte text.");

			result.resize(converted_length);
			const wchar_t * input = source;
			if (wcsrtombs_s(&converted_length, result.data(), result.size(), &input, result.size() - 1, nullptr) != 0)
				throw CoreLib::Basic::InvalidOperationException(L"Failed to convert wide string to multibyte text.");

			result.resize(converted_length - 1);
#else
			std::mbstate_t state{};
			const wchar_t * input = source;
			const auto required = std::wcsrtombs(nullptr, &input, 0, &state);
			if (required == static_cast<size_t>(-1))
				throw CoreLib::Basic::InvalidOperationException(L"Failed to convert wide string to multibyte text.");

			result.resize(required);
			state = std::mbstate_t{};
			input = source;
			std::wcsrtombs(result.data(), &input, result.size(), &state);
#endif
			return result;
		}

		std::vector<wchar_t> convert_multibyte_to_wide(const char * source)
		{
			if (!source || source[0] == 0)
				return {};

			std::vector<wchar_t> result;
#ifdef _MSC_VER
			size_t converted_length = 0;
			if (mbsrtowcs_s(&converted_length, nullptr, 0, &source, 0, nullptr) != 0 || converted_length == 0)
				throw CoreLib::Basic::InvalidOperationException(L"Failed to convert multibyte text to wide string.");

			result.resize(converted_length);
			const char * input = source;
			if (mbsrtowcs_s(&converted_length, result.data(), result.size(), &input, result.size() - 1, nullptr) != 0)
				throw CoreLib::Basic::InvalidOperationException(L"Failed to convert multibyte text to wide string.");

			result.resize(converted_length - 1);
#else
			std::mbstate_t state{};
			const char * input = source;
			const auto required = std::mbsrtowcs(nullptr, &input, 0, &state);
			if (required == static_cast<size_t>(-1))
				throw CoreLib::Basic::InvalidOperationException(L"Failed to convert multibyte text to wide string.");

			result.resize(required);
			state = std::mbstate_t{};
			input = source;
			std::mbsrtowcs(result.data(), &input, result.size(), &state);
#endif
			return result;
		}
	}

	namespace IO
	{
		using namespace CoreLib::Basic;

		class UnicodeEncoding : public Encoding
		{
		public:
			virtual std::vector<char> GetBytes(const String & str)
			{
				std::vector<char> bytes(static_cast<size_t>(str.Length()) * 2);
				if (!bytes.empty())
				{
					std::memcpy(
						bytes.data(),
						reinterpret_cast<const char*>(str.Buffer()),
						bytes.size());
				}
				return bytes;
			}

			virtual String GetString(const char * buffer, int length)
			{
				if (length < 0)
					throw ArgumentException(L"Text length cannot be negative.");
				if (buffer == nullptr)
				{
					if (length == 0)
						return String();
					throw ArgumentException(L"Text buffer cannot be null when length is positive.");
				}
				if (length == 0)
					return String();

				int startPos = 0;
				if (length >= 2)
				{
					const auto first = static_cast<unsigned char>(buffer[0]);
					const auto second = static_cast<unsigned char>(buffer[1]);
					if (first == 0xFF && second == 0xFE)
						startPos += 2;
				}

				const int remainingBytes = length - startPos;
				if (remainingBytes < 0 || (remainingBytes % 2) != 0)
					throw ArgumentException(L"Invalid UTF-16LE text format.");

				const int wcharCount = remainingBytes / 2;
				std::vector<wchar_t> wide_buffer(static_cast<size_t>(wcharCount) + 1);
				for (int i = 0; i < wcharCount; i++)
				{
					const auto low = static_cast<unsigned char>(buffer[startPos + i * 2]);
					const auto high = static_cast<unsigned char>(buffer[startPos + i * 2 + 1]);
					wide_buffer[static_cast<size_t>(i)] = static_cast<wchar_t>(low | (high << 8));
				}
				wide_buffer[static_cast<size_t>(wcharCount)] = 0;
				return String(wide_buffer.data());
			}
		};

		class AnsiEncoding : public Encoding
		{
		public:
			virtual std::vector<char> GetBytes(const String & str)
			{
				static_cast<void>(text_io_locale_initialized);
				return convert_wide_to_multibyte(str.Buffer());
			}

			virtual String GetString(const char * buffer, int length)
			{
				static_cast<void>(text_io_locale_initialized);

				if (length < 0)
					throw ArgumentException(L"Text length cannot be negative.");
				if (buffer == nullptr)
				{
					if (length == 0)
						return String();
					throw ArgumentException(L"Text buffer cannot be null when length is positive.");
				}
				if (length == 0)
					return String();

				std::vector<char> bytes(buffer, buffer + length);
				bytes.push_back(0);
				auto wide_chars = convert_multibyte_to_wide(bytes.data());
				std::vector<wchar_t> wide_buffer = wide_chars;
				wide_buffer.push_back(0);
				return String(wide_buffer.data());
			}
		};

		UnicodeEncoding __unicodeEncoding;
		AnsiEncoding __ansiEncoding;

		Encoding * Encoding::Unicode = &__unicodeEncoding;
		Encoding * Encoding::Ansi = &__ansiEncoding;

		StreamWriter::StreamWriter(const String & path, Encoding * encoding)
		{
			this->stream = new FileStream(path, FileMode::Create);
			this->encoding = encoding ? encoding : Encoding::Unicode;
			if (this->encoding == Encoding::Unicode)
			{
				const unsigned char bom[] = { 0xFF, 0xFE };
				stream->Write(bom, static_cast<int>(sizeof(bom)));
			}
		}
		StreamWriter::StreamWriter(RefPtr<Stream> stream, Encoding * encoding)
		{
			this->stream = stream;
			this->encoding = encoding ? encoding : Encoding::Unicode;
		}
		void StreamWriter::Write(const String & str)
		{
			if (encoding == Encoding::Unicode)
			{
				stream->Write(str.Buffer(), static_cast<int>(sizeof(wchar_t) * static_cast<size_t>(str.Length())));
			}
			else
			{
				auto bytes = encoding->GetBytes(String(str));
				if (!bytes.empty())
					stream->Write(bytes.data(), static_cast<int>(bytes.size()));
			}
		}
		void StreamWriter::Write(const wchar_t * str, int length)
		{
			if (str == nullptr)
				return;
			if (length < 0)
				throw ArgumentException(L"Write length cannot be negative.");
			if (length == 0 && str != nullptr)
				length = static_cast<int>(std::wcslen(str));
			if (length == 0)
				return;

			if (encoding == Encoding::Unicode)
			{
				stream->Write(str, static_cast<int>(sizeof(wchar_t) * static_cast<size_t>(length)));
			}
			else
			{
				const std::wstring wide_text(str, str + length);
				auto bytes = encoding->GetBytes(String(wide_text.c_str()));
				if (!bytes.empty())
					stream->Write(bytes.data(), static_cast<int>(bytes.size()));
			}
		}
		void StreamWriter::Write(const char * str, int length)
		{
			if (str == nullptr)
				return;
			if (length < 0)
				throw ArgumentException(L"Write length cannot be negative.");
			if (length == 0 && str != nullptr)
				length = static_cast<int>(std::strlen(str));
			if (length == 0)
				return;

			if (encoding == Encoding::Ansi)
			{
				stream->Write(str, static_cast<int>(sizeof(char) * static_cast<size_t>(length)));
			}
			else
			{
				const std::string narrow_text(str, str + length);
				String cpy(narrow_text.c_str());
				stream->Write(cpy.Buffer(), static_cast<int>(sizeof(wchar_t) * static_cast<size_t>(cpy.Length())));
			}
		}

		StreamReader::StreamReader(const String & path)
		{
			stream = new FileStream(path, FileMode::Open);
			try
			{
				ReadBuffer();
			}
			catch (EndOfStreamException)
			{
				buffer.clear();
				ptr = 0;
			}
			encoding = DetermineEncoding();
			if (encoding == nullptr)
				encoding = Encoding::Ansi;
			else if (encoding == Encoding::Unicode && buffer.size() >= 2)
			{
				const auto first = static_cast<unsigned char>(buffer[0]);
				const auto second = static_cast<unsigned char>(buffer[1]);
				if (first == 0xFF && second == 0xFE)
					ptr = 2;
			}
		}
		StreamReader::StreamReader(RefPtr<Stream> stream, Encoding * encoding)
		{
			this->stream = stream;
			this->encoding = encoding;
			try
			{
				ReadBuffer();
			}
			catch (EndOfStreamException)
			{
				buffer.clear();
				ptr = 0;
			}
			Encoding * detected_encoding = DetermineEncoding();
			if (detected_encoding != nullptr)
				this->encoding = detected_encoding;
			else if (this->encoding == nullptr)
				this->encoding = Encoding::Ansi;
			if (this->encoding == Encoding::Unicode && buffer.size() >= 2)
			{
				const auto first = static_cast<unsigned char>(buffer[0]);
				const auto second = static_cast<unsigned char>(buffer[1]);
				if (first == 0xFF && second == 0xFE)
					ptr = 2;
			}
		}

		Encoding * StreamReader::DetermineEncoding()
		{
			if (buffer.size() >= 2)
			{
				const auto first = static_cast<unsigned char>(buffer[0]);
				const auto second = static_cast<unsigned char>(buffer[1]);
				if (first == 0xFF && second == 0xFE)
					return Encoding::Unicode;
			}

			for (size_t i = 1; i < buffer.size(); i += 2)
			{
				if (buffer[i] == 0)
					return Encoding::Unicode;
			}

			return nullptr;
		}
		
		void StreamReader::ReadBuffer()
		{
			buffer.resize(4096);
			int len = stream->Read(buffer.data(), static_cast<int>(buffer.size()));
			buffer.resize(static_cast<size_t>(len));
			ptr = 0;
		}

		char StreamReader::ReadBufferChar()
		{
			if (ptr < static_cast<int>(buffer.size()))
			{
				return buffer[static_cast<size_t>(ptr++)];
			}
			ReadBuffer();
			if (ptr < static_cast<int>(buffer.size()))
			{
				return buffer[static_cast<size_t>(ptr++)];
			}
			return 0;
		}

		int StreamReader::Read(wchar_t * outputBuffer, int length)
		{
			if (length < 0)
				throw ArgumentException(L"Read length cannot be negative.");
			if (outputBuffer == nullptr || length == 0)
				return 0;
			int i = 0;
			for (i = 0; i<length; i++)
			{
				try
				{
					outputBuffer[i] = Read();
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
