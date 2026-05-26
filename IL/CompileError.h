#ifndef COMPILER_COMPILE_ERROR_H
#define COMPILER_COMPILE_ERROR_H

#include <stdexcept>
#include <string>
#include <utility>

namespace Compiler
{
	class CompileError
	{
	public:
		std::wstring Message;
		std::wstring FileName;
		int ErrorID = -1;
		int Line = -1;
		int Col = -1;

		CompileError() = default;
		CompileError(std::wstring msg, std::wstring fileName, int id, int line, int col)
			: Message(std::move(msg))
			, FileName(std::move(fileName))
			, ErrorID(id)
			, Line(line)
			, Col(col)
		{
		}
	};

	class CompileErrorException : public std::runtime_error
	{
	public:
		CompileErrorException()
			: std::runtime_error("")
		{}
		explicit CompileErrorException(const char* message)
			: std::runtime_error(message)
		{
		}
		explicit CompileErrorException(const wchar_t* message)
			: std::runtime_error(ToNarrow(std::wstring(message)))
		{
		}
		explicit CompileErrorException(const std::wstring& message)
			: std::runtime_error(ToNarrow(message))
		{
		}

	private:
		static std::string ToNarrow(const std::wstring& message)
		{
			std::string result;
			result.reserve(message.size());
			for (const wchar_t ch : message)
			{
				result.push_back(static_cast<char>(ch));
			}
			return result;
		}
	};
}

#endif
