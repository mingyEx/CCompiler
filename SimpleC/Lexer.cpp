#include "Lexer.h"
#include <stdio.h>
namespace SimpleC
{
	namespace Compiler
	{
		enum class State
		{
			Start, Identifier, Operator, Int, Double, Char, String, MultiComment, SingleComment
		};

		bool IsLetter(wchar_t ch)
		{
			return (ch >= L'a' && ch <= L'z' ||
				ch >= L'A' && ch <= L'Z' || ch == L'_');
		}

		bool IsDigit(wchar_t ch)
		{
			return ch >= L'0' && ch <= L'9';
		}

		bool IsPunctuation(wchar_t ch)	//Punctuation 标点符号
		{
			return  ch == L'+' || ch == L'-' || ch == L'*' || ch == L'/' || ch == L'%' ||
				ch == L'!' || ch == L'^' || ch == L'&' || ch == L'(' || ch == L')' ||
				ch == L'=' || ch == L'{' || ch == L'}' || ch == L'[' || ch == L']' ||
				ch == L'|' || ch == L';' || ch == L',' || ch == L'.' || ch == L'<' ||
				ch == L'>';
		}

		TokenType GetKeywordTokenType(const String& str)
		{
			if (str == L"void")
				return TokenType::KeywordVoid;
			else if (str == L"return")
				return TokenType::KeywordReturn;
			else if (str == L"break")
				return TokenType::KeywordBreak;
			else if (str == L"continue")
				return TokenType::KeywordContinue;
			else if (str == L"int")
				return TokenType::KeywordInt;
			else if (str == L"double")
				return TokenType::KeywordDouble;
			else if (str == L"char")
				return TokenType::KeywordChar;
			else if (str == L"string")
				return TokenType::KeywordString;
			else if (str == L"if")
				return TokenType::KeywordIf;
			else if (str == L"else")
				return TokenType::KeywordElse;
			else if (str == L"for")
				return TokenType::KeywordFor;
			else if (str == L"while")
				return TokenType::KeywordWhile;
			else if (str == L"do")
				return TokenType::KeywordDo;
			else
				return TokenType::Identifier;
		}

		void ParseOperators(const String& str, List<Token>& tokens, int line, int col)
		{
			int pos = 0;
			while (pos < str.Length())
			{
				wchar_t curChar = str[pos];
				wchar_t nextChar = (pos < str.Length() - 1) ? str[pos + 1] : L'\0';
				auto InsertToken = [&](TokenType type, const String& ct)
				{
					tokens.Add(Token(type, ct, line, col + pos));
				};
				switch (curChar)
				{
				case L'+':
					if (nextChar == L'+')
					{
						InsertToken(TokenType::OpInc, L"++");
						pos += 2;
					}
					else
					{
						InsertToken(TokenType::OpAdd, L"+");
						pos++;
					}
					break;
				case L'-':
					if (nextChar == L'-')
					{
						InsertToken(TokenType::OpDec, L"--");
						pos += 2;
					}
					else
					{
						InsertToken(TokenType::OpSub, L"-");
						pos++;
					}
					break;
				case L'|':
					if (nextChar == L'|')
					{
						InsertToken(TokenType::OpOr, L"||");
						pos += 2;
					}
					else
					{
						InsertToken(TokenType::OpBitOr, L"|");
						pos++;
					}
					break;
				case L'&':
					if (nextChar == L'&')
					{
						InsertToken(TokenType::OpAnd, L"&&");
						pos += 2;
					}
					else
					{
						InsertToken(TokenType::OpBitAnd, L"&");
						pos++;
					}
					break;
				case L'>':
					if (nextChar == L'>')
					{
						InsertToken(TokenType::OpRsh, L">>");
						pos += 2;
					}
					else if (nextChar == L'=')
					{
						InsertToken(TokenType::OpGeq, L">=");
						pos += 2;
					}
					else
					{
						InsertToken(TokenType::OpGreater, L">");
						pos++;
					}
					break;
				case L'<':
					if (nextChar == L'<')
					{
						InsertToken(TokenType::OpLsh, L"<<");
						pos += 2;
					}
					else if (nextChar == L'=')
					{
						InsertToken(TokenType::OpLeq, L"<=");
						pos += 2;
					}
					else
					{
						InsertToken(TokenType::OpLess, L"<");
						pos++;
					}
					break;
				case L'=':
					if (nextChar == L'=')
					{
						InsertToken(TokenType::OpEql, L"==");
						pos += 2;
					}
					else
					{
						InsertToken(TokenType::OpAssign, L"=");
						pos++;
					}
					break;
				case L'!':
					if (nextChar == L'=')
					{
						InsertToken(TokenType::OpNeq, L"!=");
						pos += 2;
					}
					else
					{
						InsertToken(TokenType::OpNot, L"!");
						pos++;
					}
					break;
				case L';':
					InsertToken(TokenType::Semicolon, L";");
					pos++;
					break;
				case L',':
					InsertToken(TokenType::Comma, L",");
					pos++;
					break;
				case L'{':
					InsertToken(TokenType::LBrace, L"{");
					pos++;
					break;
				case L'}':
					InsertToken(TokenType::RBrace, L"}");
					pos++;
					break;
				case L'[':
					InsertToken(TokenType::LBracket, L"[");
					pos++;
					break;
				case L']':
					InsertToken(TokenType::RBracket, L"]");
					pos++;
					break;
				case L'(':
					InsertToken(TokenType::LParent, L"(");
					pos++;
					break;
				case L')':
					InsertToken(TokenType::RParent, L")");
					pos++;
					break;
				case L'*':
					InsertToken(TokenType::OpMul, L"*");
					pos++;
					break;
				case L'/':
					InsertToken(TokenType::OpDiv, L"/");
					pos++;
					break;
				case L'%':
					InsertToken(TokenType::OpMod, L"%");
					pos++;
					break;
				}
			}
		}

		List<Token> Lexer::Parse(const String& fileName, const String& str, List<CompileError>& errorList)
		{
			int lastPos = 0, pos = 0;	// last是上个有效token的末尾位置，pos是当前正在分析的
			int line = 1, col = 0;
			State state = State::Start;
			StringBuilder tokenBuilder;		//返回wchar[]。
			int tokenLine, tokenCol;
			List<Token> tokenList;

			auto InsertToken = [&](TokenType type)
			{
				tokenList.Add(Token(type, tokenBuilder.ToString(), tokenLine, tokenCol));
				tokenBuilder.Clear();
			};
			auto ProcessTransferChar = [&](wchar_t nextChar)
			{
				switch (nextChar)
				{
				case L'\\':
				case L'\"':
				case L'\'':
					tokenBuilder.Append(nextChar);
					break;
				case L't':
					tokenBuilder.Append('\t');	//水平制表(HT) （跳到下一个TAB位置）
					break;
				case L's':
					tokenBuilder.Append(' ');
					break;
				case L'n':
					tokenBuilder.Append('\n');
					break;
				case L'r':
					tokenBuilder.Append('\r');	//回车(CR) ，将当前位置移到本行开头
					break;
				case L'b':
					tokenBuilder.Append('\b');	//（\b） 是将输出的定位前移一个字符的意思
					break;
				}
			};
			while (pos <= str.Length())
			{
				wchar_t curChar = (pos < str.Length() ? str[pos] : L' ');
				wchar_t nextChar = (pos < str.Length() - 1) ? str[pos + 1] : L'\0';
				if (lastPos != pos)
				{
					if (curChar == L'\n')
					{
						line++;
						col = 0;
					}
					else
					{
						col++;
					}
					lastPos = pos;	//刷新上次的位置
				}

				switch (state)
				{
				case State::Start:
					if (IsLetter(curChar))
					{
						state = State::Identifier;
						tokenLine = line;
						tokenCol = col;
					}
					else if (IsDigit(curChar))
					{
						state = State::Int;
						tokenLine = line;
						tokenCol = col;
					}
					else if (curChar == L'\'')
					{
						state = State::Char;
						pos++;
						tokenLine = line;
						tokenCol = col;
					}
					else if (curChar == L'"')
					{
						state = State::String;
						pos++;
						tokenLine = line;
						tokenCol = col;
					}
					else if (curChar == L' ' || curChar == L'\t' || curChar == L'\r' || curChar == L'\n')
						pos++;
					else if (curChar == L'/' && nextChar == L'/')
					{
						state = State::SingleComment;
						pos += 2;
					}
					else if (curChar == L'/' && nextChar == L'*')
					{
						pos += 2;
						state = State::MultiComment;
					}
					else if (IsPunctuation(curChar))
					{
						state = State::Operator;
						tokenLine = line;
						tokenCol = col;
					}
					else
					{
						errorList.Add(CompileError(L"Illegal character '" + String(curChar) + L"'", fileName, 10000, line, col));
						pos++;
					}
					break;
				case State::Identifier:
					if (IsLetter(curChar) || IsDigit(curChar))
					{
						tokenBuilder.Append(curChar);
						pos++;
					}
					else
					{
						InsertToken(GetKeywordTokenType(tokenBuilder.ToString()));
						state = State::Start;
					}
					break;
				case State::Operator:
					if (IsPunctuation(curChar) && (curChar != L'/' || nextChar != L'/'))
					{
						tokenBuilder.Append(curChar);
						pos++;
					}
					else
					{
						//do token analyze
						ParseOperators(tokenBuilder.ToString(), tokenList, tokenLine, tokenCol);
						tokenBuilder.Clear();
						state = State::Start;
					}
					break;
				case State::Int:
					if (IsDigit(curChar))
					{
						tokenBuilder.Append(curChar);
						pos++;
					}
					else if (curChar == L'.')
					{
						state = State::Double;
						tokenBuilder.Append(curChar);
						pos++;
					}
					else
					{
						InsertToken(TokenType::IntLiterial);
						state = State::Start;
					}
					break;
				case State::Double:
					if (IsDigit(curChar))
					{
						tokenBuilder.Append(curChar);
						pos++;
					}
					else
					{
						InsertToken(TokenType::DoubleLiterial);
						state = State::Start;
					}
					break;
				case State::String:
					if (curChar != L'"')
					{
						if (curChar == L'\\')
						{
							ProcessTransferChar(nextChar);
							pos++;
						}
						else
							tokenBuilder.Append(curChar);
					}
					else
					{
						InsertToken(TokenType::StringLiterial);
						state = State::Start;
					}
					pos++;
					break;
				case State::Char:
					if (curChar != L'\'')
					{
						if (curChar == L'\\')
						{
							ProcessTransferChar(nextChar);
							pos++;
						}
						else
							tokenBuilder.Append(curChar);
					}
					else
					{
						if (tokenBuilder.Length() > 1)
							errorList.Add(CompileError(L"Illegal character literial.", fileName, 10001, line, col - tokenBuilder.Length()));
						InsertToken(TokenType::CharLiterial);
						state = State::Start;
					}
					pos++;
					break;
				case State::SingleComment:
					if (curChar == L'\n')
						state = State::Start;
					pos++;
					break;
				case State::MultiComment:
					if (curChar == L'*' && nextChar == '/')
					{
						state = State::Start;
						pos += 2;
					}
					else
						pos++;
					break;
				}
			}
			return tokenList;
		}
	}
}