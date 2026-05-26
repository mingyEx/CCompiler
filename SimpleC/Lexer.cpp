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

		TokenType GetKeywordTokenType(const std::wstring& str)
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

		void ParseOperators(const std::wstring& str, std::vector<Token>& tokens, int line, int col)
		{
			const int length = static_cast<int>(str.length());
			int pos = 0;
			while (pos < length)
			{
				wchar_t curChar = str[pos];
				wchar_t nextChar = (pos < length - 1) ? str[pos + 1] : L'\0';
				auto InsertToken = [&](TokenType type, const wchar_t* ct)
				{
					tokens.emplace_back(type, ct, line, col + pos);
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

		std::vector<Token> Lexer::Parse(const std::wstring& fileName, const std::wstring& str, std::vector<CompileError>& errorList)
		{
			const int sourceLength = static_cast<int>(str.length());
			int lastPos = 0, pos = 0;	// last是上个有效token的末尾位置，pos是当前正在分析的
			int line = 1, col = 0;
			State state = State::Start;
			std::wstring tokenBuilder;
			int tokenLine, tokenCol;
			std::vector<Token> tokenList;

			auto InsertToken = [&](TokenType type)
			{
				tokenList.emplace_back(type, tokenBuilder, tokenLine, tokenCol);
				tokenBuilder.clear();
			};
			auto ProcessTransferChar = [&](wchar_t nextChar)
			{
				switch (nextChar)
				{
				case L'\\':
				case L'\"':
				case L'\'':
					tokenBuilder.push_back(nextChar);
					break;
				case L't':
					tokenBuilder.push_back(L'\t');
					break;
				case L's':
					tokenBuilder.push_back(L' ');
					break;
				case L'n':
					tokenBuilder.push_back(L'\n');
					break;
				case L'r':
					tokenBuilder.push_back(L'\r');
					break;
				case L'b':
					tokenBuilder.push_back(L'\b');
					break;
				}
			};
			while (pos <= sourceLength)
			{
				wchar_t curChar = (pos < sourceLength ? str[pos] : L' ');
				wchar_t nextChar = (pos < sourceLength - 1) ? str[pos + 1] : L'\0';
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
						std::wstring message = L"Illegal character '";
						message.push_back(curChar);
						message += L"'";
						errorList.push_back(CompileError(message, fileName, 10000, line, col));
						pos++;
					}
					break;
				case State::Identifier:
					if (IsLetter(curChar) || IsDigit(curChar))
					{
						tokenBuilder.push_back(curChar);
						pos++;
					}
					else
					{
						InsertToken(GetKeywordTokenType(tokenBuilder));
						state = State::Start;
					}
					break;
				case State::Operator:
					if (IsPunctuation(curChar) && (curChar != L'/' || nextChar != L'/'))
					{
						tokenBuilder.push_back(curChar);
						pos++;
					}
					else
					{
						//do token analyze
						ParseOperators(tokenBuilder, tokenList, tokenLine, tokenCol);
						tokenBuilder.clear();
						state = State::Start;
					}
					break;
				case State::Int:
					if (IsDigit(curChar))
					{
						tokenBuilder.push_back(curChar);
						pos++;
					}
					else if (curChar == L'.')
					{
						state = State::Double;
						tokenBuilder.push_back(curChar);
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
						tokenBuilder.push_back(curChar);
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
							tokenBuilder.push_back(curChar);
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
							tokenBuilder.push_back(curChar);
					}
					else
					{
						if (tokenBuilder.length() > 1)
							errorList.push_back(CompileError(L"Illegal character literial.", fileName, 10001, line, col - static_cast<int>(tokenBuilder.length())));
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
