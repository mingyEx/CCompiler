#ifndef SIMPLE_C_LEXER_H
#define SIMPLE_C_LEXER_H

#include "IL/CompileError.h"
#include <string>
#include <utility>
#include <vector>

namespace SimpleC
{
	namespace Compiler
	{
		enum class TokenType
		{
			// illegal
			Unkown,
			// identifier
			Identifier,
			KeywordVoid, KeywordReturn, KeywordBreak, KeywordContinue,
			KeywordInt, KeywordDouble, KeywordChar, KeywordString,
			KeywordIf, KeywordElse, KeywordFor, KeywordWhile, KeywordDo,
			// constant
			IntLiterial, DoubleLiterial, StringLiterial, CharLiterial,
			// operators
			Semicolon, Comma, LBrace, RBrace, LBracket, RBracket, LParent, RParent,
			OpAssign, OpAdd, OpSub, OpMul, OpDiv, OpMod, OpNot, OpLsh, OpRsh, 
			OpEql, OpNeq, OpGreater, OpLess, OpGeq, OpLeq,
			OpAnd, OpOr, OpBitXor, OpBitAnd, OpBitOr,
			OpInc, OpDec
		};

		class Token
		{
		public:
			TokenType Type;
			std::wstring Content;
			int Line, Col;

			Token()
			{
				Type = TokenType::Unkown;
				Line = Col = -1;
			}

			Token(TokenType type, std::wstring content, int line, int col)
				: Type(type)
				, Content(std::move(content))
				, Line(line)
				, Col(col)
			{}
		};

		class Lexer
		{
		public:
			std::vector<Token> Parse(const std::wstring & fileName, const std::wstring & str, std::vector<::Compiler::CompileError> & errorList);
		};
	}
}

#endif
