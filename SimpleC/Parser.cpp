#include "Parser.h"

namespace SimpleC
{
	namespace Compiler
	{
		Token & Parser::ReadToken(TokenType type)
		{
			if (pos >= tokens.Count())
			{
				errors.Add(CompileError(TokenTypeToString(type) + String(L" expected but end of file encountered."), fileName, 0, 0, 0));
				throw 0;
			}
			else if(tokens[pos].Type != type)
			{
				errors.Add(CompileError(TokenTypeToString(type) + String(L" expected"), fileName, 20001, tokens[pos].Line, tokens[pos].Col));
				throw 20001;
			}
			return tokens[pos++];
		}

		bool Parser::LookAheadToken(TokenType type)
		{
			if (pos >= tokens.Count())
			{
				errors.Add(CompileError(TokenTypeToString(type) + String(L" expected but end of file encountered."), fileName, 0, 0, 0));
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
			if (pos >= tokens.Count())
			{
				errors.Add(CompileError(String(L"type name expected but end of file encountered."), fileName, 0, tokens[pos].Line, tokens[pos].Col));
				throw 0;
			}
			if(!IsTypeKeyword())
			{
				errors.Add(CompileError(String(L"type name expected but end of file encountered."), fileName, 20001, tokens[pos].Line, tokens[pos].Col));
				throw 20001;
			}
			return tokens[pos++];
		}

		bool Parser::IsTypeKeyword()
		{
			if (pos >= tokens.Count())
			{
				errors.Add(CompileError(String(L"TypeKeyword expected but end of file encountered."), fileName, 0, tokens[pos].Line, tokens[pos].Col));
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

		String Parser::TokenTypeToString(TokenType type)
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

		RefPtr<ProgramSyntaxNode> Parser::Parse()
		{
			return ParseProgram();
		}

		RefPtr<ProgramSyntaxNode> Parser::ParseProgram()
		{
			RefPtr<ProgramSyntaxNode> program = new ProgramSyntaxNode();
			try
			{
				while(pos < tokens.Count())
				{
					program->Functions.Add(ParseFunction());
				}
			}
			catch(int){}
			return program;
		}

		RefPtr<FunctionSyntaxNode> Parser::ParseFunction()
		{
			RefPtr<FunctionSyntaxNode> function = new FunctionSyntaxNode();
			function->ReturnType = ParseType();
			try
			{
				Token & name = ReadToken(TokenType::Identifier);
				function->Name = name.Content;
				function->Line = name.Line;
				function->FileName = fileName;
				function->Col = name.Col;
				ReadToken(TokenType::LParent);
				while(pos < tokens.Count() && tokens[pos].Type != TokenType::RParent)
				{
					function->Parameters.Add(ParseParameter());
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
				while (pos < tokens.Count() && tokens[pos].Type != TokenType::LBrace)
				{
					pos++;
				}
			}
			function->Body = ParseBlockStatement();
			return function;
		}

		RefPtr<StatementSyntaxNode> Parser::ParseStatement()
		{
			RefPtr<StatementSyntaxNode> statement;
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
				statement = new EmptyStatementSyntaxNode();
				statement->Line = tokens[pos].Line;
				statement->Col = tokens[pos].Col;
				statement->FileName = fileName;
				ReadToken(TokenType::Semicolon);
			}
			else
			{
				errors.Add(CompileError(String(L"Syntax error."), fileName, 20002, tokens[pos].Line, tokens[pos].Col));
				throw 20002;
			}
			return statement;
		}

		RefPtr<BlockStatementSyntaxNode> Parser::ParseBlockStatement()
		{
			RefPtr<BlockStatementSyntaxNode> blockStatement = new BlockStatementSyntaxNode();
			ReadToken(TokenType::LBrace);
			try
			{
				if(pos < tokens.Count())
				{
					blockStatement->Line = tokens[pos].Line;
					blockStatement->Col = tokens[pos].Col;
					blockStatement->FileName = fileName;
				}

				while(pos < tokens.Count() && !LookAheadToken(TokenType::RBrace))
					blockStatement->Statements.Add(ParseStatement());
			}
			catch(int e)
			{
				if(e == 0)
					return blockStatement;
				while(pos < tokens.Count() && tokens[pos].Type != TokenType::RBrace)
					pos++;
			}
			ReadToken(TokenType::RBrace);
			return blockStatement;
		}

		RefPtr<VarDeclrStatementSyntaxNode> Parser::ParseVarDeclrStatement()
		{
			RefPtr<VarDeclrStatementSyntaxNode>varDeclrStatement = new VarDeclrStatementSyntaxNode();
			try
			{
				varDeclrStatement->Type = ParseType();
				varDeclrStatement->Line = varDeclrStatement->Type->Line;
				varDeclrStatement->Col = varDeclrStatement->Type->Col;
				varDeclrStatement->FileName = fileName;
				while(pos < tokens.Count())
				{
					Token & name = ReadToken(TokenType::Identifier);
					RefPtr<VarDeclrStatementSyntaxNode::Variable> var = new VarDeclrStatementSyntaxNode::Variable();
					var->Name = name.Content;
					var->Line = name.Line;
					var->Col = name.Col;
					var->FileName = fileName;
					if (LookAheadToken(TokenType::OpAssign))
					{
						ReadToken(TokenType::OpAssign);
						var->Expression = ParseExpression();
					}

					varDeclrStatement->Variables.Add(var);
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
				while (pos < tokens.Count() && tokens[pos].Type != TokenType::Semicolon)
					pos++;
			}
			return varDeclrStatement;
		}

		RefPtr<IfStatementSyntaxNode> Parser::ParseIfStatement()
		{
			RefPtr<IfStatementSyntaxNode> ifStatement = new IfStatementSyntaxNode();
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

		RefPtr<ForStatementSyntaxNode> Parser::ParseForStatement()
		{
			RefPtr<ForStatementSyntaxNode> stmt = new ForStatementSyntaxNode();
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

		RefPtr<WhileStatementSyntaxNode> Parser::ParseWhileStatement()
		{
			RefPtr<WhileStatementSyntaxNode> whileStatement = new WhileStatementSyntaxNode();
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

		RefPtr<DoWhileStatementSyntaxNode> Parser::ParseDoWhileStatement()
		{
			RefPtr<DoWhileStatementSyntaxNode> doWhileStatement = new DoWhileStatementSyntaxNode();
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

		RefPtr<BreakStatementSyntaxNode> Parser::ParseBreakStatement()
		{
			RefPtr<BreakStatementSyntaxNode> breakStatement = new BreakStatementSyntaxNode();
			Token & breakToken = ReadToken(TokenType::KeywordBreak);
			breakStatement->Line = breakToken.Line;
			breakStatement->Col = breakToken.Col;
			breakStatement->FileName = fileName;
			ReadToken(TokenType::Semicolon);
			return breakStatement;
		}

		RefPtr<ContinueStatementSyntaxNode>	Parser::ParseContinueStatement()
		{
			RefPtr<ContinueStatementSyntaxNode> continueStatement = new ContinueStatementSyntaxNode();
			Token & token = ReadToken(TokenType::KeywordContinue);
			continueStatement->Line = token.Line;
			continueStatement->Col = token.Col;
			continueStatement->FileName = fileName;
			ReadToken(TokenType::Semicolon);
			return continueStatement;
		}

		RefPtr<ReturnStatementSyntaxNode> Parser::ParseReturnStatement()
		{
			RefPtr<ReturnStatementSyntaxNode> returnStatement = new ReturnStatementSyntaxNode();
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

		RefPtr<ExpressionStatementSyntaxNode> Parser::ParseExpressionStatement()
		{
			RefPtr<ExpressionStatementSyntaxNode> returnStatement = new ExpressionStatementSyntaxNode();
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
				while(pos < tokens.Count() && (tokens[pos].Type != TokenType::Semicolon || tokens[pos].Type != TokenType::RBrace))
					pos++;
			}
			ReadToken(TokenType::Semicolon);
			return returnStatement;
		}

		RefPtr<ParameterSyntaxNode> Parser::ParseParameter()
		{
			RefPtr<ParameterSyntaxNode> parameter = new ParameterSyntaxNode();
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
				while (pos < tokens.Count() && tokens[pos].Type != TokenType::Identifier)
					pos++;
				if(pos < tokens.Count())
					parameter->Name = tokens[pos].Content;
			}
			return parameter;
		}

		RefPtr<TypeSyntaxNode> Parser::ParseType()
		{
			RefPtr<TypeSyntaxNode> type = new TypeSyntaxNode();
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
				while (pos < tokens.Count() && !IsTypeKeyword())
					pos++;
				if(pos < tokens.Count())
					type->TypeName = tokens[pos].Content;
			}
			if(LookAheadToken(TokenType::LBracket))
			{
				ReadToken(TokenType::LBracket);
				type->IsArray = true;
				type->ArrayLength = atoi(ReadToken(TokenType::IntLiterial).Content.ToMultiByteString());
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

		RefPtr<ExpressionSyntaxNode> Parser::ParseExpression(int level)
		{
			if (level == MaxExprLevel)
				return ParseLeafExpression();

			if (GetAssociativityFromLevel(level) == Associativity::Left)
			{
				auto left = ParseExpression(level + 1);
				while(pos < tokens.Count() && GetOpLevel(tokens[pos].Type) == level)
				{
					RefPtr<BinaryExpressionSyntaxNode> tmp = new BinaryExpressionSyntaxNode();
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
				if (pos < tokens.Count() && GetOpLevel(tokens[pos].Type) == level)
				{
					RefPtr<BinaryExpressionSyntaxNode> tmp = new BinaryExpressionSyntaxNode();
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

		RefPtr<ExpressionSyntaxNode> Parser::ParseLeafExpression()
		{
			if (LookAheadToken(TokenType::LParent))
			{
				ReadToken(TokenType::LParent);
				RefPtr<ExpressionSyntaxNode> expr;
				try
				{
					expr = ParseExpression();
				}
				catch(int e)
				{
					if (e = 0)
						return expr;
					while (pos < tokens.Count() && 
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
				RefPtr<UnaryExpressionSyntaxNode> unaryExpr = new UnaryExpressionSyntaxNode();
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

			RefPtr<ExpressionSyntaxNode> rs;

			if (LookAheadToken(TokenType::IntLiterial) ||
				LookAheadToken(TokenType::DoubleLiterial) ||
				LookAheadToken(TokenType::CharLiterial) ||
				LookAheadToken(TokenType::StringLiterial))
			{
				RefPtr<ConstantExpressionSyntaxNode> constExpr = new ConstantExpressionSyntaxNode();
				auto token = tokens[pos++];
				constExpr->Line = token.Line;
				constExpr->Col = token.Col;
				constExpr->FileName = fileName;
				if (token.Type == TokenType::IntLiterial)
				{
					constExpr->ConstType = ConstantExpressionSyntaxNode::ConstantType::Int;
					constExpr->IntValue = StringToInt(token.Content);
				}
				else if (token.Type == TokenType::DoubleLiterial)
				{
					constExpr->ConstType = ConstantExpressionSyntaxNode::ConstantType::Double;
					constExpr->DoubleValue = StringToDouble(token.Content);
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
				RefPtr<VarExpressionSyntaxNode> varExpr = new VarExpressionSyntaxNode();
				Token & token = ReadToken(TokenType::Identifier);
				varExpr->Variable = token.Content; 
				varExpr->Line = token.Line;
				varExpr->Col = token.Col;
				varExpr->FileName = fileName;
				rs = varExpr;
			}
			while (pos < tokens.Count() &&
					(LookAheadToken(TokenType::OpInc) ||
					LookAheadToken(TokenType::OpDec) ||
					LookAheadToken(TokenType::LBracket) ||
					LookAheadToken(TokenType::LParent)))
			{
				if (LookAheadToken(TokenType::OpInc))
				{
					RefPtr<UnaryExpressionSyntaxNode> unaryExpr = new UnaryExpressionSyntaxNode();
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
					RefPtr<UnaryExpressionSyntaxNode> unaryExpr = new UnaryExpressionSyntaxNode();
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
					RefPtr<IndexExpressionSyntaxNode> indexExpr = new IndexExpressionSyntaxNode();
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
						while (pos < tokens.Count() && 
							(tokens[pos].Type != TokenType::RBracket))
							pos++;
					}
					ReadToken(TokenType::RBracket);
					rs = indexExpr;
				}
				else if (LookAheadToken(TokenType::LParent))	//圆括号，函数调用语句
				{
					RefPtr<InvokeExpressionSyntaxNode> invokeExpr = new InvokeExpressionSyntaxNode();
					invokeExpr->FunctionExpr = rs;
					Token & token = ReadToken(TokenType::LParent);
					invokeExpr->Line = token.Line;
					invokeExpr->Col = token.Col;
					invokeExpr->FileName = fileName;
					while (pos < tokens.Count())
					{
						try
						{
							if (!LookAheadToken(TokenType::RParent))
								invokeExpr->Arguments.Add(ParseExpression());
							else
							{
								break;
							}
						}
						catch (int e)
						{
							if (e == 0)
								return rs;
							while (pos < tokens.Count() && 
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
				if (pos < tokens.Count())
				{
					line = tokens[pos].Line;
					col = tokens[pos].Col;
				}
				errors.Add(CompileError(String(L"Syntax error."), fileName, 20005, line, col)); 
				throw 20005;
			}
			return rs;
		}
	}
}