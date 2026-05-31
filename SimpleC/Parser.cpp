#include "Parser.h"
#include <cwchar>

namespace SimpleC
{
	namespace Compiler
	{
		using ::Compiler::CompileError;

		Token & Parser::ReadToken(TokenType type)
		{
			if (pos >= TokenCount())
			{
				errors.push_back(CompileError(TokenTypeToString(type) + L" expected but end of file encountered.", fileName, 0, 0, 0));
				throw 0;
			}
			else if(tokens[pos].Type != type)
			{
				errors.push_back(CompileError(TokenTypeToString(type) + L" expected", fileName, 20001, tokens[pos].Line, tokens[pos].Col));
				throw 20001;
			}
			return tokens[pos++];
		}

		bool Parser::LookAheadToken(TokenType type)
		{
			if (pos >= TokenCount())
			{
				errors.push_back(CompileError(TokenTypeToString(type) + L" expected but end of file encountered.", fileName, 0, 0, 0));
				return false;
			}
			else
			{
				if(tokens[pos].Type == type)
					return true;
				else
					return false;
			}
		}

		Token & Parser::ReadTypeKeyword()
		{
			if (pos >= TokenCount())
			{
				errors.push_back(CompileError(L"type name expected but end of file encountered.", fileName, 0, tokens[pos].Line, tokens[pos].Col));
				throw 0;
			}
			if(!IsTypeKeyword())
			{
				errors.push_back(CompileError(L"type name expected but end of file encountered.", fileName, 20001, tokens[pos].Line, tokens[pos].Col));
				throw 20001;
			}
			return tokens[pos++];
		}

		bool Parser::IsTypeKeyword()
		{
			if (pos >= TokenCount())
			{
				errors.push_back(CompileError(L"TypeKeyword expected but end of file encountered.", fileName, 0, tokens[pos].Line, tokens[pos].Col));
				throw 0;
			}

			if (tokens[pos].Type != TokenType::KeywordInt &&
				tokens[pos].Type != TokenType::KeywordDouble &&
				tokens[pos].Type != TokenType::KeywordChar &&
				tokens[pos].Type != TokenType::KeywordString &&
				tokens[pos].Type != TokenType::KeywordVoid)
				return false;
			else	//支持的类型
				return true;
		}

		std::wstring Parser::TokenTypeToString(TokenType type)
		{
			switch (type)
			{
			case TokenType::Unkown:
				return L"UnknownToken";
			case TokenType::Identifier:	
				return L"Identifier";
			case TokenType::KeywordVoid:
				return L"\"void\"";
			case TokenType::KeywordReturn:
				return L"\"return\"";
			case TokenType::KeywordBreak:
				return L"\"break\"";
			case TokenType::KeywordContinue:
				return L"\"continue\"";
			case TokenType::KeywordInt:
				return L"\"int\"";
			case TokenType::KeywordDouble:
				return L"\"double\"";
			case TokenType::KeywordChar:
				return L"\"char\"";
			case TokenType::KeywordString:
				return L"\"string\"";
			case TokenType::KeywordIf:
				return L"\"if\"";
			case TokenType::KeywordElse:
				return L"\"else\"";
			case TokenType::KeywordFor:
				return L"\"for\"";
			case TokenType::KeywordWhile:
				return L"\"while\"";
			case TokenType::KeywordDo:
				return L"\"do\"";
			case TokenType::IntLiterial:	//字符字面量的值
				return L"Int Literial";
			case TokenType::DoubleLiterial:
				return L"Double Literial";
			case TokenType::StringLiterial:
				return L"String Literial";
			case TokenType::CharLiterial:
				return L"CharLiterial";
			case TokenType::Semicolon:
				return L"';'";
			case TokenType::Comma:
				return L"','";
			case TokenType::LBrace:
				return L"'{'";
			case TokenType::RBrace:
				return L"'}'";
			case TokenType::LBracket:
				return L"'['";
			case TokenType::RBracket:
				return L"']'";
			case TokenType::LParent:
				return L"'('";
			case TokenType::RParent:
				return L"')'";
				break;
			case TokenType::OpAssign:
				return L"'='";
			case TokenType::OpAdd:
				return L"'+'";
			case TokenType::OpSub:
				return L"'-'";
			case TokenType::OpMul:
				return L"'*'";
			case TokenType::OpDiv:
				return L"'/'";
			case TokenType::OpMod:
				return L"'%'";
			case TokenType::OpNot:
				return L"'!'";
			case TokenType::OpLsh:
				return L"'<<'";
			case TokenType::OpRsh:
				return L"'>>'";
				break;
			case TokenType::OpEql:
				return L"'=='";
			case TokenType::OpNeq:
				return L"'!='";
			case TokenType::OpGreater:
				return L"'>'";
			case TokenType::OpLess:
				return L"'<'";
			case TokenType::OpGeq:
				return L"'>='";
			case TokenType::OpLeq:
				return L"'<='";
			case TokenType::OpAnd:
				return L"'&&'";
			case TokenType::OpOr:
				return L"'||'";
			case TokenType::OpBitXor:
				return L"'^'";
			case TokenType::OpBitAnd:
				return L"'&'";
			case TokenType::OpBitOr:
				return L"'|'";
			case TokenType::OpInc:
				return L"'++'";
			case TokenType::OpDec:
				return L"'--'";
			default:
				return L"";
			}
		}

		std::shared_ptr<ProgramSyntaxNode> Parser::Parse()
		{
			return ParseProgram();
		}

		std::shared_ptr<ProgramSyntaxNode> Parser::ParseProgram()
		{
			auto program = std::make_shared<ProgramSyntaxNode>();
			try
			{
				while(pos < TokenCount())
				{
					program->Functions.push_back(ParseFunction());
				}
			}
			catch(int){}
			return program;
		}

		std::shared_ptr<FunctionSyntaxNode> Parser::ParseFunction()
		{
			auto function = std::make_shared<FunctionSyntaxNode>();
			function->ReturnType = ParseType();
			try
			{
				Token & name = ReadToken(TokenType::Identifier);
				function->Name = name.Content;
				function->Line = name.Line;
				function->FileName = fileName;
				function->Col = name.Col;
				ReadToken(TokenType::LParent);
				while(pos < TokenCount() && tokens[pos].Type != TokenType::RParent)
				{
					function->Parameters.push_back(ParseParameter());
					if(LookAheadToken(TokenType::Comma))
						ReadToken(TokenType::Comma);
					else
						break;
				}
				ReadToken(TokenType::RParent);
			}
			catch(int e)
			{
				if (e == 0)
					return function;
				while (pos < TokenCount() && tokens[pos].Type != TokenType::LBrace)
				{
					pos++;
				}
			}
			function->Body = ParseBlockStatement();
			return function;
		}

		std::shared_ptr<StatementSyntaxNode> Parser::ParseStatement()
		{
			std::shared_ptr<StatementSyntaxNode> statement;
			if (LookAheadToken(TokenType::LBrace))
				statement = ParseBlockStatement();
			else if (IsTypeKeyword())
				statement = ParseVarDeclrStatement();
			else if (LookAheadToken(TokenType::KeywordIf))
				statement = ParseIfStatement();
			else if (LookAheadToken(TokenType::KeywordFor))
				statement = ParseForStatement();
			else if (LookAheadToken(TokenType::KeywordWhile))
				statement = ParseWhileStatement();
			else if (LookAheadToken(TokenType::KeywordDo))
				statement = ParseDoWhileStatement();
			else if (LookAheadToken(TokenType::KeywordBreak))
				statement = ParseBreakStatement();
			else if (LookAheadToken(TokenType::KeywordContinue))
				statement = ParseContinueStatement();
			else if (LookAheadToken(TokenType::KeywordReturn))
				statement = ParseReturnStatement();
			else if (LookAheadToken(TokenType::Identifier))
				statement = ParseExpressionStatement();
			else if (LookAheadToken(TokenType::Semicolon))
			{
				statement = std::make_shared<EmptyStatementSyntaxNode>();
				statement->Line = tokens[pos].Line;
				statement->Col = tokens[pos].Col;
				statement->FileName = fileName;
				ReadToken(TokenType::Semicolon);
			}
			else
			{
				errors.push_back(CompileError(L"Syntax error.", fileName, 20002, tokens[pos].Line, tokens[pos].Col));
				throw 20002;
			}
			return statement;
		}

		std::shared_ptr<BlockStatementSyntaxNode> Parser::ParseBlockStatement()
		{
			auto blockStatement = std::make_shared<BlockStatementSyntaxNode>();
			ReadToken(TokenType::LBrace);
			try
			{
				if(pos < TokenCount())
				{
					blockStatement->Line = tokens[pos].Line;
					blockStatement->Col = tokens[pos].Col;
					blockStatement->FileName = fileName;
				}

				while(pos < TokenCount() && !LookAheadToken(TokenType::RBrace))
					blockStatement->Statements.push_back(ParseStatement());
			}
			catch(int e)
			{
				if(e == 0)
					return blockStatement;
				while(pos < TokenCount() && tokens[pos].Type != TokenType::RBrace)
					pos++;
			}
			ReadToken(TokenType::RBrace);
			return blockStatement;
		}

		std::shared_ptr<VarDeclrStatementSyntaxNode> Parser::ParseVarDeclrStatement()
		{
			auto varDeclrStatement = std::make_shared<VarDeclrStatementSyntaxNode>();
			try
			{
				varDeclrStatement->Type = ParseType();
				varDeclrStatement->Line = varDeclrStatement->Type->Line;
				varDeclrStatement->Col = varDeclrStatement->Type->Col;
				varDeclrStatement->FileName = fileName;
				while(pos < TokenCount())
				{
					Token & name = ReadToken(TokenType::Identifier);
					auto var = std::make_shared<VarDeclrStatementSyntaxNode::Variable>();
					var->Name = name.Content;
					var->Line = name.Line;
					var->Col = name.Col;
					var->FileName = fileName;
					if (LookAheadToken(TokenType::OpAssign))
					{
						ReadToken(TokenType::OpAssign);
						var->Expression = ParseExpression();
					}

					varDeclrStatement->Variables.push_back(var);
					if (LookAheadToken(TokenType::Comma))
						ReadToken(TokenType::Comma);
					else
						break;
				}
				ReadToken(TokenType::Semicolon);
			}
			catch(int e)
			{
				if (e == 0)
					return varDeclrStatement;
				while (pos < TokenCount() && tokens[pos].Type != TokenType::Semicolon)
					pos++;
			}
			return varDeclrStatement;
		}

		std::shared_ptr<IfStatementSyntaxNode> Parser::ParseIfStatement()
		{
			auto ifStatement = std::make_shared<IfStatementSyntaxNode>();
			Token & ifToken = ReadToken(TokenType::KeywordIf);
			ifStatement->Line = ifToken.Line;
			ifStatement->Col = ifToken.Col;
			ifStatement->FileName = fileName;
			ReadToken(TokenType::LParent);
			ifStatement->Predicate = ParseExpression();
			ReadToken(TokenType::RParent);
			ifStatement->PositiveStatement = ParseStatement();
			if (LookAheadToken(TokenType::KeywordElse))
			{
				ReadToken(TokenType::KeywordElse);
				ifStatement->NegativeStatement = ParseStatement();
			}
			return ifStatement;
		}

		std::shared_ptr<ForStatementSyntaxNode> Parser::ParseForStatement()
		{
			auto stmt = std::make_shared<ForStatementSyntaxNode>();
			Token & token = ReadToken(TokenType::KeywordFor);
			stmt->Line = token.Line;
			stmt->Col = token.Col;
			stmt->FileName = fileName;
			ReadToken(TokenType::LParent);
			if(IsTypeKeyword())
				stmt->VarDeclr = ParseVarDeclrStatement();
			else
			{
				if(!LookAheadToken(TokenType::Semicolon))
					stmt->InitialExpression = ParseExpression();
				ReadToken(TokenType::Semicolon);
			}
			if(!LookAheadToken(TokenType::Semicolon))
				stmt->MarginExpression = ParseExpression();
			ReadToken(TokenType::Semicolon);
			if(!LookAheadToken(TokenType::RParent))
				stmt->SideEffectExpression = ParseExpression();
			ReadToken(TokenType::RParent);
			stmt->Statement = ParseStatement();

			return stmt;
		}

		std::shared_ptr<WhileStatementSyntaxNode> Parser::ParseWhileStatement()
		{
			auto whileStatement = std::make_shared<WhileStatementSyntaxNode>();
			Token & whileToken = ReadToken(TokenType::KeywordWhile);
			whileStatement->Line = whileToken.Line;
			whileStatement->Col = whileToken.Col;
			whileStatement->FileName = fileName;
			ReadToken(TokenType::LParent);
			whileStatement->Predicate = ParseExpression();
			ReadToken(TokenType::RParent);
			whileStatement->Statement = ParseStatement();
			return whileStatement;
		}

		std::shared_ptr<DoWhileStatementSyntaxNode> Parser::ParseDoWhileStatement()
		{
			auto doWhileStatement = std::make_shared<DoWhileStatementSyntaxNode>();
			Token & doToken = ReadToken(TokenType::KeywordDo);
			doWhileStatement->Line = doToken.Line;
			doWhileStatement->Col = doToken.Col;
			doWhileStatement->FileName = fileName;
			doWhileStatement->Statement = ParseStatement();
			ReadToken(TokenType::KeywordWhile);
			ReadToken(TokenType::LParent);
			doWhileStatement->Predicate = ParseExpression();
			ReadToken(TokenType::RParent);
			ReadToken(TokenType::Semicolon);
			return doWhileStatement;
		}

		std::shared_ptr<BreakStatementSyntaxNode> Parser::ParseBreakStatement()
		{
			auto breakStatement = std::make_shared<BreakStatementSyntaxNode>();
			Token & breakToken = ReadToken(TokenType::KeywordBreak);
			breakStatement->Line = breakToken.Line;
			breakStatement->Col = breakToken.Col;
			breakStatement->FileName = fileName;
			ReadToken(TokenType::Semicolon);
			return breakStatement;
		}

		std::shared_ptr<ContinueStatementSyntaxNode>	Parser::ParseContinueStatement()
		{
			auto continueStatement = std::make_shared<ContinueStatementSyntaxNode>();
			Token & token = ReadToken(TokenType::KeywordContinue);
			continueStatement->Line = token.Line;
			continueStatement->Col = token.Col;
			continueStatement->FileName = fileName;
			ReadToken(TokenType::Semicolon);
			return continueStatement;
		}

		std::shared_ptr<ReturnStatementSyntaxNode> Parser::ParseReturnStatement()
		{
			auto returnStatement = std::make_shared<ReturnStatementSyntaxNode>();
			Token & token = ReadToken(TokenType::KeywordReturn);
			returnStatement->Line = token.Line;
			returnStatement->Col = token.Col;
			returnStatement->FileName = fileName;
			if (!LookAheadToken(TokenType::Semicolon))	
				//如果return 后面不是 ;就收拾掉return xx的xx.
				returnStatement->Expression = ParseExpression();
			ReadToken(TokenType::Semicolon);
			return returnStatement;
		}

		std::shared_ptr<ExpressionStatementSyntaxNode> Parser::ParseExpressionStatement()
		{
			auto returnStatement = std::make_shared<ExpressionStatementSyntaxNode>();
			try
			{
				returnStatement->Expression = ParseExpression();
				if (returnStatement->Expression)
				{
					returnStatement->Line = returnStatement->Expression->Line;
					returnStatement->Col = returnStatement->Expression->Col;
					returnStatement->FileName = fileName;
				}
			}
			catch (int)
			{
					while(pos < TokenCount() && (tokens[pos].Type != TokenType::Semicolon || tokens[pos].Type != TokenType::RBrace))
					pos++;
			}
			ReadToken(TokenType::Semicolon);
			return returnStatement;
		}

		std::shared_ptr<ParameterSyntaxNode> Parser::ParseParameter()
		{
			auto parameter = std::make_shared<ParameterSyntaxNode>();
			try
			{
				parameter->Type = ParseType();
				Token & name = ReadToken(TokenType::Identifier);
				parameter->Name = name.Content;
				parameter->Line = name.Line;
				parameter->Col = name.Col;
				parameter->FileName = fileName;
			}
			catch(int e)
			{
				if(e == 0)
					return parameter;
				while (pos < TokenCount() && tokens[pos].Type != TokenType::Identifier)
					pos++;
				if(pos < TokenCount())
					parameter->Name = tokens[pos].Content;
			}
			return parameter;
		}

		std::shared_ptr<TypeSyntaxNode> Parser::ParseType()
		{
			auto type = std::make_shared<TypeSyntaxNode>();
			try
			{
				type->Line = tokens[pos].Line;
				type->Col = tokens[pos].Col;
				type->FileName = fileName;
				type->TypeName = ReadTypeKeyword().Content;
			}
			catch(int e)
			{
				if(e == 0)
					return type;
				while (pos < TokenCount() && !IsTypeKeyword())
					pos++;
				if(pos < TokenCount())
					type->TypeName = tokens[pos].Content;
			}
			if(LookAheadToken(TokenType::LBracket))
			{
				ReadToken(TokenType::LBracket);
				type->IsArray = true;
				type->ArrayLength = static_cast<int>(std::wcstol(ReadToken(TokenType::IntLiterial).Content.c_str(), nullptr, 10));
				ReadToken(TokenType::RBracket);
			}
			return type;
		}

		enum class Associativity
		{
			Left, Right
		};

		Associativity GetAssociativityFromLevel(int level)
		{
			if (level == 0)
				return Associativity::Right;
			else
				return Associativity::Left;
		}

		int GetOpLevel(TokenType type)
		{
			switch(type)
			{
			case TokenType::OpAssign:
				return 0;
			case TokenType::OpOr:
				return 1;
			case TokenType::OpAnd:
				return 2;
			case TokenType::OpBitOr:
				return 3;
			case TokenType::OpBitXor:
				return 4;
			case TokenType::OpBitAnd:
				return 5;
			case TokenType::OpEql:
			case TokenType::OpNeq:
				return 6;
			case TokenType::OpGeq:
			case TokenType::OpLeq:
			case TokenType::OpGreater:
			case TokenType::OpLess:
				return 7;
			case TokenType::OpLsh:
			case TokenType::OpRsh:
				return 8;
			case TokenType::OpAdd:
			case TokenType::OpSub:
				return 9;
			case TokenType::OpMul:
			case TokenType::OpDiv:
			case TokenType::OpMod:
				return 10;
			default:
				return -1;
			}
		}

		Operator GetOpFromToken(Token & token)	//运算符优先级
		{
			switch(token.Type)
			{
			case TokenType::OpAssign:
				return Operator::Assign;
			case TokenType::OpOr:
				return Operator::Or;
			case TokenType::OpAnd:
				return Operator::And;
			case TokenType::OpBitOr:
				return Operator::BitOr;
			case TokenType::OpBitXor:
				return Operator::BitXor;
			case TokenType::OpBitAnd:
				return Operator::BitAnd;
			case TokenType::OpEql:
				return Operator::Eql;
			case TokenType::OpNeq:
				return Operator::Neq;
			case TokenType::OpGeq:
				return Operator::Geq;
			case TokenType::OpLeq:
				return Operator::Leq;
			case TokenType::OpGreater:
				return Operator::Greater;
			case TokenType::OpLess:
				return Operator::Less;
			case TokenType::OpLsh:
				return Operator::Lsh;
			case TokenType::OpRsh:
				return Operator::Rsh;
			case TokenType::OpAdd:
				return Operator::Add;
			case TokenType::OpSub:
				return Operator::Sub;
			case TokenType::OpMul:
				return Operator::Mul;
			case TokenType::OpDiv:
				return Operator::Div;
			case TokenType::OpMod:
				return Operator::Mod;
			case TokenType::OpInc:
				return Operator::PostInc;
			case TokenType::OpDec:
				return Operator::PostDec;
			case TokenType::OpNot:
				return Operator::Not;
			default:
				throw L"Illegal TokenType.";
			}
		}

		std::shared_ptr<ExpressionSyntaxNode> Parser::ParseExpression(int level)
		{
			if (level == MaxExprLevel)
				return ParseLeafExpression();

			if (GetAssociativityFromLevel(level) == Associativity::Left)
			{
				auto left = ParseExpression(level + 1);
				while(pos < TokenCount() && GetOpLevel(tokens[pos].Type) == level)
				{
					auto tmp = std::make_shared<BinaryExpressionSyntaxNode>();
					tmp->LeftExpression = left;
					Token & opToken = ReadToken(tokens[pos].Type);
					tmp->Operator = GetOpFromToken(opToken);
					tmp->Line = opToken.Line;
					tmp->Col = opToken.Col;
					tmp->RightExpression = ParseExpression(level + 1);
					left = tmp;
				}
				return left;
			}
			else
			{
				auto left = ParseExpression(level + 1);
				if (pos < TokenCount() && GetOpLevel(tokens[pos].Type) == level)
				{
					auto tmp = std::make_shared<BinaryExpressionSyntaxNode>();
					tmp->LeftExpression = left;
					Token & opToken = ReadToken(tokens[pos].Type);
					tmp->Operator = GetOpFromToken(opToken);
					tmp->Line = opToken.Line;
					tmp->Col = opToken.Col;
					tmp->RightExpression = ParseExpression(level);
					left = tmp;
				}
				return left;
			}
		}

		std::shared_ptr<ExpressionSyntaxNode> Parser::ParseLeafExpression()
		{
			if (LookAheadToken(TokenType::LParent))
			{
				ReadToken(TokenType::LParent);
				std::shared_ptr<ExpressionSyntaxNode> expr;
				try
				{
					expr = ParseExpression();
				}
				catch(int e)
				{
					if (e == 0)
						return expr;
					while (pos < TokenCount() && 
						(tokens[pos].Type != TokenType::RParent))
						pos++;
				}
				ReadToken(TokenType::RParent);
				return expr;
			}

			if (LookAheadToken(TokenType::OpInc) ||
				LookAheadToken(TokenType::OpDec) ||
				LookAheadToken(TokenType::OpNot) ||
				LookAheadToken(TokenType::OpSub))	// OpSub 减法subtractions 而不是subscript
			{
				auto unaryExpr = std::make_shared<UnaryExpressionSyntaxNode>();
				Token & token = tokens[pos++];
				unaryExpr->Line = token.Line;
				unaryExpr->Col = token.Col;
				unaryExpr->FileName = fileName;
				unaryExpr->Operator = GetOpFromToken(token);	//Token本身就是Op,而非其中一个对象.
				if (unaryExpr->Operator == Operator::PostInc)
					unaryExpr->Operator = Operator::PreInc;
				else if (unaryExpr->Operator == Operator::PostDec)
					unaryExpr->Operator = Operator::PreDec;
				else if (unaryExpr->Operator == Operator::Sub)
					unaryExpr->Operator = Operator::Neg;

				unaryExpr->Expression = ParseLeafExpression();	//操作符后面的表达式递归下降进行处理。
				return unaryExpr;
			}

			std::shared_ptr<ExpressionSyntaxNode> rs;

			if (LookAheadToken(TokenType::IntLiterial) ||
				LookAheadToken(TokenType::DoubleLiterial) ||
				LookAheadToken(TokenType::CharLiterial) ||
				LookAheadToken(TokenType::StringLiterial))
			{
				auto constExpr = std::make_shared<ConstantExpressionSyntaxNode>();
				auto token = tokens[pos++];
				constExpr->Line = token.Line;
				constExpr->Col = token.Col;
				constExpr->FileName = fileName;
				if (token.Type == TokenType::IntLiterial)
				{
					constExpr->ConstType = ConstantExpressionSyntaxNode::ConstantType::Int;
					constExpr->IntValue = static_cast<int>(std::wcstol(token.Content.c_str(), nullptr, 10));
				}
				else if (token.Type == TokenType::DoubleLiterial)
				{
					constExpr->ConstType = ConstantExpressionSyntaxNode::ConstantType::Double;
					constExpr->DoubleValue = std::wcstod(token.Content.c_str(), nullptr);
				}
				else if (token.Type == TokenType::CharLiterial)
				{
					constExpr->ConstType = ConstantExpressionSyntaxNode::ConstantType::Char;
					constExpr->CharValue = (wchar_t)token.Content[0];
				}
				else
				{
					constExpr->ConstType = ConstantExpressionSyntaxNode::ConstantType::String;
					constExpr->StringValue = token.Content;
				}
				rs = constExpr;
			}
			else if (LookAheadToken(TokenType::Identifier))
			{
				auto varExpr = std::make_shared<VarExpressionSyntaxNode>();
				Token & token = ReadToken(TokenType::Identifier);
				varExpr->Variable = token.Content; 
				varExpr->Line = token.Line;
				varExpr->Col = token.Col;
				varExpr->FileName = fileName;
				rs = varExpr;
			}
			while (pos < TokenCount() &&
					(LookAheadToken(TokenType::OpInc) ||
					LookAheadToken(TokenType::OpDec) ||
					LookAheadToken(TokenType::LBracket) ||
					LookAheadToken(TokenType::LParent)))
			{
				if (LookAheadToken(TokenType::OpInc))
				{
					auto unaryExpr = std::make_shared<UnaryExpressionSyntaxNode>();
					Token & token = ReadToken(TokenType::OpInc);
					unaryExpr->Line = token.Line;
					unaryExpr->Col = token.Col;
					unaryExpr->FileName = fileName;
					unaryExpr->Operator = Operator::PostInc;
					unaryExpr->Expression = rs;
					rs = unaryExpr;
				}
				else if (LookAheadToken(TokenType::OpDec))
				{
					auto unaryExpr = std::make_shared<UnaryExpressionSyntaxNode>();
					Token & token = ReadToken(TokenType::OpDec);
					unaryExpr->Line = token.Line;
					unaryExpr->Col = token.Col;
					unaryExpr->FileName = fileName;
					unaryExpr->Operator = Operator::PostDec;
					unaryExpr->Expression = rs;
					rs = unaryExpr;
				}
				else if (LookAheadToken(TokenType::LBracket))		//bracket 方括号
				{
					auto indexExpr = std::make_shared<IndexExpressionSyntaxNode>();
					indexExpr->BaseExpression = rs;	//arr[i] 的arr
					Token & token = ReadToken(TokenType::LBracket);
					indexExpr->Line = token.Line;
					indexExpr->Col = token.Col;
					indexExpr->FileName = fileName;
					try
					{
						indexExpr->IndexExpression = ParseExpression();	//arr[i]的i部分，i可能表达式.
					}
					catch(int e)
					{
						if (e == 0)
							return rs;
						while (pos < TokenCount() && 
							(tokens[pos].Type != TokenType::RBracket))
							pos++;
					}
					ReadToken(TokenType::RBracket);
					rs = indexExpr;
				}
				else if (LookAheadToken(TokenType::LParent))	//圆括号，函数调用语句
				{
					auto invokeExpr = std::make_shared<InvokeExpressionSyntaxNode>();
					invokeExpr->FunctionExpr = rs;
					Token & token = ReadToken(TokenType::LParent);
					invokeExpr->Line = token.Line;
					invokeExpr->Col = token.Col;
					invokeExpr->FileName = fileName;
					while (pos < TokenCount())
					{
						try
						{
							if (!LookAheadToken(TokenType::RParent))
					invokeExpr->Arguments.push_back(ParseExpression());
							else
							{
								break;
							}
						}
						catch (int e)
						{
							if (e == 0)
								return rs;
							while (pos < TokenCount() && 
								(tokens[pos].Type != TokenType::Comma && tokens[pos].Type != TokenType::RParent))
								pos++;
						}
						if (!LookAheadToken(TokenType::Comma))
							break;
						ReadToken(TokenType::Comma);
					}
					ReadToken(TokenType::RParent);
					rs = invokeExpr;
				}
			}
			if (!rs)
			{
				int line = 0, col = 0;
				if (pos < TokenCount())
				{
					line = tokens[pos].Line;
					col = tokens[pos].Col;
				}
				errors.push_back(CompileError(L"Syntax error.", fileName, 20005, line, col)); 
				throw 20005;
			}
			return rs;
		}
	}
}
