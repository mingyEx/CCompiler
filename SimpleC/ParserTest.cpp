#include "Parser.h"

namespace SimpleC
{
	namespace Compiler
	{
		SmartPointer<ProgramSyntaxNode> Parser::ParseProgram()
		{
			SmartPointer<ProgramSyntaxNode> program = new ProgramSyntaxNode();
			while(pos < tokens.Count())
				program->Functions.Add(ParseFunction());
			return program;
		}

		SmartPointer<FunctionSyntaxNode> Parser::ParseFunction()
		{
			SmartPointer<FunctionSyntaxNode> function = new FunctionSyntaxNode();
			try
			{
				function->ReturnType = ParseType();
				Token & name = ReadToken(TokenType::Identifier);
				function->Line = name.Line;
				function->Col = name.Col;
				function->Name = name.Content;
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
				if(e == 0)
					return function;
				while(pos < tokens.Count() && tokens[pos].Type != TokenType::LBrace)
					pos++;
			}
			function->Body = ParseBlockStatement();
			return function;
		}


	}
}