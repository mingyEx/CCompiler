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
			res.buffer = new wchar_t[res.length + 1];
			wcscpy_s(res.buffer.Ptr(), res.length + 1, lhs);
			wcscpy_s(res.buffer + leftLen, res.length + 1 - leftLen, rhs);
			return res;
		}
		String operator+(const wchar_t * op1, const String & op2)
		{
			if(!op2.buffer)
				return String(op1);

			return StringConcat(op1, wcslen(op1), op2.buffer.Ptr(), op2.length);
		}

		String operator+(const String & op1, const wchar_t*op2)
		{
			if(!op1.buffer)
				return String(op2);

			return StringConcat(op1.buffer.Ptr(), op1.length, op2, wcslen(op2));
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
			return (int)_wcstoi64(str.Buffer(), NULL, 10);
		}
		double StringToDouble(const String & str)
		{
			return (double)wcstod(str.Buffer(), NULL);
		}
	}
}
