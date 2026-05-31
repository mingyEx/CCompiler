#ifndef SIMPLE_C_PARSER_H
#define SIMPLE_C_PARSER_H

#include "Lexer.h"
#include "Syntax.h"
#include <string>
#include <utility>
#include <vector>

namespace SimpleC
{
	namespace Compiler
	{
		const int MaxExprLevel = 11;

		class Parser
		{
		private:
			int pos;
			std::vector<Token>& tokens;
			std::vector<::Compiler::CompileError>& errors;
			std::wstring fileName;
			int TokenCount() const
			{
				return static_cast<int>(tokens.size());
			}
		public:
			Parser(std::vector<Token>& _tokens, std::vector<::Compiler::CompileError>& _errors, std::wstring _fileName)
				:tokens(_tokens), errors(_errors), pos(0), fileName(std::move(_fileName))
			{}
			std::shared_ptr<ProgramSyntaxNode> Parse();	//一个返回该程序根节点的parse函数.
		private:
			Token& ReadToken(TokenType type);
			bool LookAheadToken(TokenType type);
			Token& ReadTypeKeyword();
			bool IsTypeKeyword();
			std::wstring TokenTypeToString(TokenType type);
			std::shared_ptr<ProgramSyntaxNode> ParseProgram();
			std::shared_ptr<FunctionSyntaxNode> ParseFunction();
			std::shared_ptr<StatementSyntaxNode>			ParseStatement();
			std::shared_ptr<BlockStatementSyntaxNode>		ParseBlockStatement();
			std::shared_ptr<VarDeclrStatementSyntaxNode>	ParseVarDeclrStatement();
			std::shared_ptr<IfStatementSyntaxNode>			ParseIfStatement();
			std::shared_ptr<ForStatementSyntaxNode>		ParseForStatement();
			std::shared_ptr<WhileStatementSyntaxNode>		ParseWhileStatement();
			std::shared_ptr<DoWhileStatementSyntaxNode>	ParseDoWhileStatement();
			std::shared_ptr<BreakStatementSyntaxNode>		ParseBreakStatement();
			std::shared_ptr<ContinueStatementSyntaxNode>	ParseContinueStatement();
			std::shared_ptr<ReturnStatementSyntaxNode>		ParseReturnStatement();
			std::shared_ptr<ExpressionStatementSyntaxNode> ParseExpressionStatement();
			std::shared_ptr<ExpressionSyntaxNode>			ParseExpression(int level = 0);
			std::shared_ptr<ExpressionSyntaxNode>		ParseLeafExpression();
			std::shared_ptr<ParameterSyntaxNode> ParseParameter();
			std::shared_ptr<TypeSyntaxNode> ParseType();
		};
	}
}

#endif
