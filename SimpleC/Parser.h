#ifndef SIMPLE_C_PARSER_H
#define SIMPLE_C_PARSER_H

#include "Lexer.h"
#include "Syntax.h"

namespace SimpleC
{
	namespace Compiler
	{
		const int MaxExprLevel = 11;

		class Parser
		{
		private:
			int pos;
			List<Token>& tokens;
			List<CompileError>& errors;
			String fileName;
		public:
			Parser(List<Token>& _tokens, List<CompileError>& _errors, String _fileName)
				:tokens(_tokens), errors(_errors), pos(0), fileName(_fileName)
			{}
			RefPtr<ProgramSyntaxNode> Parse();	//一个返回该程序根节点的parse函数.
		private:
			Token& ReadToken(TokenType type);
			bool LookAheadToken(TokenType type);
			Token& ReadTypeKeyword();
			bool IsTypeKeyword();
			String TokenTypeToString(TokenType type);
			RefPtr<ProgramSyntaxNode> ParseProgram();
			RefPtr<FunctionSyntaxNode> ParseFunction();
			RefPtr<StatementSyntaxNode>			ParseStatement();
			RefPtr<BlockStatementSyntaxNode>		ParseBlockStatement();
			RefPtr<VarDeclrStatementSyntaxNode>	ParseVarDeclrStatement();
			RefPtr<IfStatementSyntaxNode>			ParseIfStatement();
			RefPtr<ForStatementSyntaxNode>		ParseForStatement();
			RefPtr<WhileStatementSyntaxNode>		ParseWhileStatement();
			RefPtr<DoWhileStatementSyntaxNode>	ParseDoWhileStatement();
			RefPtr<BreakStatementSyntaxNode>		ParseBreakStatement();
			RefPtr<ContinueStatementSyntaxNode>	ParseContinueStatement();
			RefPtr<ReturnStatementSyntaxNode>		ParseReturnStatement();
			RefPtr<ExpressionStatementSyntaxNode> ParseExpressionStatement();
			RefPtr<ExpressionSyntaxNode>			ParseExpression(int level = 0);
			RefPtr<ExpressionSyntaxNode>		ParseLeafExpression();
			RefPtr<ParameterSyntaxNode> ParseParameter();
			RefPtr<TypeSyntaxNode> ParseType();
		};
	}
}

#endif