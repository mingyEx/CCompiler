#ifndef SIMPLE_C_LEXER_H
#define SIMPLE_C_LEXER_H

#include "Basic.h"
#include "IL/CompileError.h"

namespace SimpleC
{
	namespace Compiler
	{
		using namespace CoreLib::Basic;
		using namespace ::Compiler;

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
			String Content;
			int Line, Col;

			Token()
			{
				Type = TokenType::Unkown;
				Line = Col = -1;
			}

			Token(TokenType type, const String & content, int line, int col)
			{
				Type = type;
				Content = content;
				Line = line;
				Col = col;
			}
		};

		class Lexer
		{
		public:
			List<Token> Parse(const String & fileName, const String & str, List<CompileError> & errorList);
		};
	}
}

#endif