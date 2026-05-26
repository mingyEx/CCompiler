#ifndef FUNDAMENTAL_LIB_STRING_H
#define FUNDAMENTAL_LIB_STRING_H
#include <clocale>
#include <cwchar>
#include <cwctype>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "SmartPointer.h"

namespace CoreLib
{
	namespace Basic
	{
		class _EndLine		{};
		extern _EndLine EndLine;
		class String
		{
			friend class StringBuilder;
		private:
			static int WriteIntToBuffer(int value, int radix, wchar_t * destination, size_t capacity)
			{
				if (!destination || capacity == 0)
					return 0;

				if (radix < 2 || radix > 36)
				{
					destination[0] = 0;
					return 0;
				}

				wchar_t tmp[65];
				int index = 0;
				unsigned int magnitude = 0;
				const bool negative = value < 0;
				if (negative)
					magnitude = static_cast<unsigned int>(-(static_cast<long long>(value)));
				else
					magnitude = static_cast<unsigned int>(value);

				do
				{
					const unsigned int digit = magnitude % static_cast<unsigned int>(radix);
					tmp[index++] = digit < 10 ? static_cast<wchar_t>(L'0' + digit) : static_cast<wchar_t>(L'a' + digit - 10);
					magnitude /= static_cast<unsigned int>(radix);
				}
				while (magnitude != 0 && index < static_cast<int>(std::size(tmp) - 1));

				if (negative && index < static_cast<int>(std::size(tmp) - 1))
					tmp[index++] = L'-';

				const size_t resultLength = static_cast<size_t>(index);
				if (resultLength + 1 > capacity)
				{
					destination[0] = 0;
					return 0;
				}

				for (int i = 0; i < index; i++)
					destination[i] = tmp[index - 1 - i];
				destination[index] = 0;
				return index;
			}

			static std::vector<wchar_t> ConvertMultiByteToWide(const char * source)
			{
				static_cast<void>(localeInitialized);
				if (!source || source[0] == 0)
					return {};

				std::vector<wchar_t> result;
#ifdef _MSC_VER
				size_t convertedLength = 0;
				if (mbsrtowcs_s(&convertedLength, nullptr, 0, &source, 0, nullptr) != 0 || convertedLength == 0)
					throw "String: invalid multibyte input.";

				result.resize(convertedLength);
				const char * input = source;
				if (mbsrtowcs_s(&convertedLength, result.data(), result.size(), &input, result.size() - 1, nullptr) != 0)
					throw "String: invalid multibyte input.";

				result.resize(convertedLength - 1);
#else
				std::mbstate_t state{};
				const char * input = source;
				const auto required = std::mbsrtowcs(nullptr, &input, 0, &state);
				if (required == static_cast<size_t>(-1))
					throw "String: invalid multibyte input.";

				result.resize(required);
				state = std::mbstate_t{};
				input = source;
				std::mbsrtowcs(result.data(), &input, result.size(), &state);
#endif
				return result;
			}

			static std::vector<char> ConvertWideToMultiByte(const wchar_t * source)
			{
				static_cast<void>(localeInitialized);
				if (!source || source[0] == 0)
					return {};

				std::vector<char> result;
#ifdef _MSC_VER
				size_t convertedLength = 0;
				if (wcsrtombs_s(&convertedLength, nullptr, 0, &source, 0, nullptr) != 0 || convertedLength == 0)
					return {};

				result.resize(convertedLength);
				const wchar_t * input = source;
				if (wcsrtombs_s(&convertedLength, result.data(), result.size(), &input, result.size() - 1, nullptr) != 0)
					return {};

				result.resize(convertedLength - 1);
#else
				std::mbstate_t state{};
				const wchar_t * input = source;
				const auto required = std::wcsrtombs(nullptr, &input, 0, &state);
				if (required == static_cast<size_t>(-1))
					return {};

				result.resize(required);
				state = std::mbstate_t{};
				input = source;
				std::wcsrtombs(result.data(), &input, result.size(), &state);
#endif
				return result;
			}

			template<typename T>
			static std::wstring FormatFloatingPoint(T value, const wchar_t * format)
			{
				std::wostringstream stream;
				stream.imbue(std::locale::classic());

				const wchar_t * effectiveFormat = format ? format : L"%e";
				int precision = 6;
				wchar_t style = L'e';

				if (effectiveFormat[0] == L'%' && effectiveFormat[1] != 0)
				{
					const wchar_t * cursor = effectiveFormat + 1;
					if (*cursor == L'.')
					{
						precision = 0;
						++cursor;
						while (*cursor >= L'0' && *cursor <= L'9')
						{
							precision = precision * 10 + (*cursor - L'0');
							++cursor;
						}
					}
					if (*cursor != 0)
						style = static_cast<wchar_t>(::towlower(*cursor));
				}

				stream << std::setprecision(precision);
				switch (style)
				{
				case L'f':
					stream << std::fixed;
					break;
				case L'g':
					stream << std::defaultfloat;
					break;
				case L'e':
				default:
					stream << std::scientific;
					break;
				}

				stream << value;
				return stream.str();
			}

			static bool IsTrimWhitespace(wchar_t ch)
			{
				return std::iswspace(static_cast<wint_t>(ch)) != 0;
			}

			inline static const bool localeInitialized = []()
			{
				std::setlocale(LC_ALL, "");
				return true;
			}();

			RefPtr<wchar_t, RefPtrArrayDestructor> buffer;
			mutable std::string multi_byte_buffer;
			int length;
			void Free()
			{
				buffer = nullptr;
				multi_byte_buffer.clear();
				length = 0;
			}

		public:
			static String FromBuffer(RefPtr<wchar_t, RefPtrArrayDestructor> buffer, int len)
			{
				String rs;
				rs.buffer = buffer;
				rs.length = len;
				return rs;
			}
			String()
				:buffer(nullptr), length(0)
			{
			}
			String(const wchar_t * str)
				:buffer(nullptr), length(0)
			{
				this->operator=(str);
			}
			String(const wchar_t ch)
				:buffer(nullptr), length(0)
			{
				wchar_t arr[] = {ch, 0};
				*this = String(arr);
			}
			String(int val, int radix = 10)
				:buffer(nullptr), length(0)
			{
				buffer = new wchar_t[static_cast<size_t>(33)];
				length = WriteIntToBuffer(val, radix, buffer.Ptr(), 33);
			}
			String(float val, const wchar_t * format = L"%e")
				:buffer(nullptr), length(0)
			{
				const auto formatted = FormatFloatingPoint(val, format);
				length = static_cast<int>(formatted.size());
				buffer = new wchar_t[static_cast<size_t>(length) + 1];
				wcscpy_s(buffer.Ptr(), static_cast<size_t>(length) + 1, formatted.c_str());
			}
			String(double val, const wchar_t * format = L"%e")
				:buffer(nullptr), length(0)
			{
				const auto formatted = FormatFloatingPoint(val, format);
				length = static_cast<int>(formatted.size());
				buffer = new wchar_t[static_cast<size_t>(length) + 1];
				wcscpy_s(buffer.Ptr(), static_cast<size_t>(length) + 1, formatted.c_str());
			}
			String(const char * str)
				:buffer(nullptr), length(0)
			{
				auto wideChars = ConvertMultiByteToWide(str);
				length = static_cast<int>(wideChars.size());
				if (length != 0)
				{
					buffer = new wchar_t[static_cast<size_t>(length) + 1];
					std::memcpy(buffer.Ptr(), wideChars.data(), sizeof(wchar_t) * wideChars.size());
					buffer[length] = 0;
				}
			}
			String(const String & str)
				:buffer(nullptr), length(0)
			{				
				this->operator=(str);
			}
			String(String&& other)
				:buffer(nullptr), length(0)
			{
				this->operator=(std::move(other));
			}
			~String()
			{
				Free();
			}
			String & operator=(const wchar_t * str)
			{
				if (str == nullptr)
				{
					Free();
					return *this;
				}
				const std::wstring source(str);
				Free();
				length = static_cast<int>(source.size());
				buffer = new wchar_t[static_cast<size_t>(length) + 1];
				wcscpy_s(buffer.Ptr(), static_cast<size_t>(length) + 1, source.c_str());
				return *this;
			}
			String & operator=(const String & str)
			{
				if (str.buffer == buffer)
					return *this;
				Free();
				if (str.buffer)
				{
					length = str.length;
					buffer = str.buffer;
					multi_byte_buffer.clear();
				}
				return *this;
			}
			String & operator=(String&& other)
			{
				if (this != &other)
				{
					Free();
					buffer = std::move(other.buffer);
					length = other.length;
					multi_byte_buffer = std::move(other.multi_byte_buffer);
					other.buffer = nullptr;
					other.length = 0;
					other.multi_byte_buffer.clear();
				}
				return *this;
			}
			wchar_t operator[](int id) const
			{
#if _DEBUG
				if (id < 0 || id >= length)
					throw "Operator[]: index out of range.";
#endif
				if (id < 0 || id >= length)
					return 0;
				return buffer.Ptr()[id];
			}

			friend String StringConcat(const wchar_t * lhs, int leftLen, const wchar_t * rhs, int rightLen);
			friend String operator+(const wchar_t*op1, const String & op2);
			friend String operator+(const String & op1, const wchar_t * op2);
			friend String operator+(const String & op1, const String & op2);

			String TrimStart() const
			{
				if(!buffer)
					return *this;
				int startIndex = 0;
				while (startIndex < length && IsTrimWhitespace(buffer[startIndex]))
						startIndex++;
				if (startIndex >= length)
					return String();
				return String(buffer + startIndex);
			}

			String TrimEnd() const
			{
				if(!buffer)
					return *this;

				int endIndex = length - 1;
				while (endIndex >= 0 && IsTrimWhitespace(buffer[endIndex]))
					endIndex--;
				if (endIndex < 0)
					return String();
				String res;
				res.length = endIndex + 1;
				res.buffer = new wchar_t[static_cast<size_t>(res.length) + 1];
				wcsncpy_s(res.buffer.Ptr(), static_cast<size_t>(res.length) + 1, buffer.Ptr(), static_cast<size_t>(res.length));
				return res;
			}

			String Trim() const
			{
				if(!buffer)
					return *this;

				int startIndex = 0;
				while (startIndex < length && IsTrimWhitespace(buffer[startIndex]))
						startIndex++;
				int endIndex = length - 1;
				while (endIndex >= startIndex && IsTrimWhitespace(buffer[endIndex]))
					endIndex--;
				if (startIndex > endIndex)
					return String();

				String res;
				res.length = endIndex - startIndex + 1;
				res.buffer = new wchar_t[static_cast<size_t>(res.length) + 1];
				std::memcpy(res.buffer.Ptr(), buffer + startIndex, sizeof(wchar_t) * static_cast<size_t>(res.length));
				res.buffer[res.length] = L'\0';
				return res;
			}

			String SubString(int id, int len) const
			{
#if _DEBUG
				if (len < 0)
					throw "SubString: length less than zero.";
				if (id < 0 || id > length || (id + len) > length)
					throw "SubString: index out of range.";
#endif
				if (len < 0)
					return String();
				if (id < 0 || id > length || (id + len) > length)
					return String();
				if (len == 0)
					return String();
				String res;
				res.buffer = new wchar_t[static_cast<size_t>(len) + 1];
				res.length = len;
				wcsncpy_s(res.buffer.Ptr(), static_cast<size_t>(len) + 1, buffer + id, static_cast<size_t>(len));
				return res;
			}

			const wchar_t * Buffer() const
			{
				if (buffer)
					return buffer.Ptr();
				else
					return L"";
			}

			const char * ToMultiByteString(int * len = 0) const
			{
				if (!buffer)
				{
					if (len)
						*len = 0;
					return "";
				}
				else
				{
					if (!multi_byte_buffer.empty())
					{
						if (len)
							*len = static_cast<int>(multi_byte_buffer.size());
						return multi_byte_buffer.c_str();
					}

					auto multiByteChars = ConvertWideToMultiByte(buffer.Ptr());
					if (multiByteChars.empty())
					{
						if (len)
							*len = 0;
						return "";
					}

					if (len)
						*len = static_cast<int>(multiByteChars.size());
					multi_byte_buffer.assign(multiByteChars.begin(), multiByteChars.end());
					return multi_byte_buffer.c_str();
				}
			}

			bool operator==(const String & str) const
			{
				return std::wcscmp(Buffer(), str.Buffer()) == 0;
			}
			bool operator!=(const String & str) const
			{
				return std::wcscmp(Buffer(), str.Buffer()) != 0;
			}
			bool operator>(const String & str) const
			{
				return std::wcscmp(Buffer(), str.Buffer()) > 0;
			}
			bool operator<(const String & str) const
			{
				return std::wcscmp(Buffer(), str.Buffer()) < 0;
			}
			bool operator>=(const String & str) const
			{
				int res = std::wcscmp(Buffer(), str.Buffer());
				return (res > 0 || res == 0);
			}
			bool operator<=(const String & str) const
			{
				int res = std::wcscmp(Buffer(), str.Buffer());
				return (res < 0 || res == 0);
			}

			String ToUpper() const
			{
				if(!buffer)
					return *this;
				String res;
				res.length = length;
				res.buffer = new wchar_t[static_cast<size_t>(length) + 1];
				for (int i = 0; i <= length; i++)
				{
					res.buffer[i] = static_cast<wchar_t>(std::towupper(buffer[i]));
				}
				return res;
			}

			String ToLower() const
			{
				if(!buffer)
					return *this;
				String res;
				res.length = length;
				res.buffer = new wchar_t[static_cast<size_t>(length) + 1];
				for (int i = 0; i <= length; i++)
				{
					res.buffer[i] = static_cast<wchar_t>(std::towlower(buffer[i]));
				}
				return res;
			}
			
			int Length() const
			{
				return length;
			}

			int IndexOf(const wchar_t * str, int id) const // String str
			{
#if _DEBUG
				if (id < 0 || id > length)
					throw "SubString: index out of range.";
#endif
				if (id < 0 || id > length)
					return -1;
				if (str == nullptr)
					return -1;
				const wchar_t * found = std::wcsstr(Buffer() + id, str);
				if (!found)
					return -1;
				return static_cast<int>(found - Buffer());
			}
			
			int IndexOf(const String & str, int id) const
			{
				return IndexOf(str.Buffer(), id);
			}

			int IndexOf(const wchar_t * str) const
			{
				return IndexOf(str, 0);
			}

			int IndexOf(const String & str) const
			{
				return IndexOf(str.Buffer(), 0);
			}

			int IndexOf(wchar_t ch, int id) const
			{
#if _DEBUG
				if (id < 0 || id > length)
					throw "SubString: index out of range.";
#endif
				if (id < 0 || id > length)
					return -1;
				const wchar_t * found = std::wcschr(Buffer() + id, ch);
				if (!found)
					return -1;
				return static_cast<int>(found - Buffer());
			}

			int IndexOf(wchar_t ch) const
			{
				return IndexOf(ch, 0);
			}

			int LastIndexOf(wchar_t ch) const
			{
				const wchar_t * source = Buffer();
				for (int i = length-1; i>=0; i--)
					if (source[i] == ch)
						return i;
				return -1;
			}

			bool StartsWith(const wchar_t * str) const // String str
			{
				if (str == nullptr)
					return false;
				const int strLen = static_cast<int>(std::wcslen(str));
				if (strLen > length)
					return false;
				const wchar_t * source = Buffer();
				for (int i = 0; i < strLen; i++)
					if (str[i] != source[i])
						return false;
				return true;
			}

			bool StartWith(const String & str) const
			{
				return StartsWith(str.Buffer());
			}

			bool EndsWith(const wchar_t * str)  const // String str
			{
				if (str == nullptr)
					return false;
				const int strLen = static_cast<int>(std::wcslen(str));
				if (strLen > length)
					return false;
				const wchar_t * source = Buffer();
				for (int i = strLen - 1; i >= 0; i--)
					if (str[i] != source[length - strLen + i])
						return false;
				return true;
			}

			bool EndsWith(const String & str) const
			{
				return EndsWith(str.Buffer());
			}

			bool Contains(const wchar_t * str) const // String str
			{
				if (str == nullptr)
					return false;
				return (IndexOf(str) >= 0)? true : false;
			}

			bool Contains(const String & str) const
			{
				return Contains(str.Buffer());
			}

			int GetHashCode() const
			{
				if (!buffer)
					return 0;
				int hash = 0;
				int c = 0;
				wchar_t * str = buffer.Ptr();
				while ((c = *str++) != 0)
					hash = c + (hash << 6) + (hash << 16) - hash;
				return hash;
			}
		};

		class StringBuilder
		{
		private:
			std::vector<wchar_t> buffer;
			int length;	
			static const int InitialSize = 512;
		public:
			StringBuilder(int bufferSize = 1024)
				:length(0)
			{
				int initialCapacity = bufferSize + 1;
				if (initialCapacity < InitialSize)
					initialCapacity = InitialSize;
				buffer.resize(static_cast<size_t>(initialCapacity));
				buffer[0] = L'\0';
			}
			void EnsureCapacity(int size)
			{
				if (static_cast<int>(buffer.size()) <= size)
					buffer.resize(static_cast<size_t>(size) + 1, L'\0');
			}

			StringBuilder & operator << (const wchar_t * str)
			{
				if (str != nullptr)
					Append(str, static_cast<int>(std::wcslen(str)));
				return *this;
			}
			StringBuilder & operator << (const String & str)
			{
				Append(str);
				return *this;
			}
			StringBuilder & operator << (const _EndLine &)
			{
				Append(L'\n');
				return *this;
			}
			void Append(wchar_t ch)
			{
				Append(&ch, 1);
			}
			void Append(int value, int radix = 10)
			{
				wchar_t localBuffer[33];
				String::WriteIntToBuffer(value, radix, localBuffer, 33);
				Append(localBuffer);
			}
			void Append(const String & str)
			{
				Append(str.Buffer(), str.Length());
			}
			void Append(const wchar_t * str)
			{
				if (str != nullptr)
					Append(str, static_cast<int>(std::wcslen(str)));
			}
			void Append(const wchar_t * str, int strLen)
			{
				if (str == nullptr || strLen <= 0)
					return;
				int newLength = length + strLen;
				if (static_cast<int>(buffer.size()) < newLength + 1)
				{
					int newBufferSize = static_cast<int>(buffer.size());
					if (newBufferSize < InitialSize)
						newBufferSize = InitialSize;
					while (newBufferSize < newLength + 1)
						newBufferSize <<= 1;
					buffer.resize(static_cast<size_t>(newBufferSize), L'\0');
				}
				std::memcpy(buffer.data() + length, str, sizeof(wchar_t) * static_cast<size_t>(strLen));
				buffer[static_cast<size_t>(newLength)] = L'\0';
				length = newLength;
			}

			int Capacity()
			{
				return static_cast<int>(buffer.size());
			}

			wchar_t * Buffer()
			{
				return buffer.data();
			}

			int Length()
			{
				return length;
			}

			String ToString()
			{
				return String(buffer.data());
			}

			String ProduceString()
			{
				String rs;
				rs.buffer = new wchar_t[static_cast<size_t>(length) + 1];
				std::memcpy(rs.buffer.Ptr(), buffer.data(), sizeof(wchar_t) * static_cast<size_t>(length + 1));
				rs.length = length;
				buffer.clear();
				buffer.resize(static_cast<size_t>(InitialSize));
				buffer[0] = L'\0';
				length = 0;
				return rs;
			}

			void Remove(int id, int len)
			{
#if _DEBUG
				if (id >= length || id < 0)
					throw "Remove: Index out of range.";
				if(len < 0)
					throw "Remove: remove length smaller than zero.";
#endif
				if (id < 0 || id >= length || len <= 0)
					return;
				int actualDelLength = ((id + len) >= length)? (length - id) : len;
				for (int i = id + actualDelLength; i <= length; i++)
					buffer[static_cast<size_t>(i - actualDelLength)] = buffer[static_cast<size_t>(i)];
				length -= actualDelLength;
			}

			void Clear()
			{
				length = 0;
				if (!buffer.empty())
					buffer[0] = 0;
			}
		};

		int StringToInt(const String & str);
		double StringToDouble(const String & str);
	}
}

#endif
