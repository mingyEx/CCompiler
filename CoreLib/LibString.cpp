#include "LibString.h"
namespace CoreLib
{
	namespace Basic
	{
		_EndLine EndLine;
		String StringConcat(const wchar_t * lhs, int leftLen, const wchar_t * rhs, int rightLen)
		{
			String res;
			res.length = leftLen + rightLen;
			res.buffer = new wchar_t[static_cast<size_t>(res.length) + 1];
			if (leftLen > 0)
				std::memcpy(res.buffer.Ptr(), lhs, sizeof(wchar_t) * static_cast<size_t>(leftLen));
			if (rightLen > 0)
				std::memcpy(res.buffer.Ptr() + leftLen, rhs, sizeof(wchar_t) * static_cast<size_t>(rightLen));
			res.buffer[res.length] = 0;
			return res;
		}
		String operator+(const wchar_t * op1, const String & op2)
		{
			if (op1 == nullptr)
				return String(op2);
			if(!op2.buffer)
				return String(op1);

			return StringConcat(op1, static_cast<int>(std::wcslen(op1)), op2.buffer.Ptr(), op2.length);
		}

		String operator+(const String & op1, const wchar_t*op2)
		{
			if (op2 == nullptr)
				return String(op1);
			if(!op1.buffer)
				return String(op2);

			return StringConcat(op1.buffer.Ptr(), op1.length, op2, static_cast<int>(std::wcslen(op2)));
		}

		String operator+(const String & op1, const String & op2)
		{
			if(!op1.buffer && !op2.buffer)
				return String();
			else if(!op1.buffer)
				return String(op2);
			else if(!op2.buffer)
				return String(op1);

			return StringConcat(op1.buffer.Ptr(), op1.length, op2.buffer.Ptr(), op2.length);
		}

		int StringToInt(const String & str)
		{
			return static_cast<int>(_wcstoi64(str.Buffer(), nullptr, 10));
		}
		double StringToDouble(const String & str)
		{
			return static_cast<double>(std::wcstod(str.Buffer(), nullptr));
		}
	}
}
