#include "Syntax.h"
#include "SyntaxVisitors.h"

namespace SimpleC
{
	namespace Compiler
	{
		ExpressionType ExpressionType::Int(ExpressionType::_Int);
		ExpressionType ExpressionType::Double(ExpressionType::_Double);
		ExpressionType ExpressionType::Char(ExpressionType::_Char);
		ExpressionType ExpressionType::String(ExpressionType::_String);
		ExpressionType ExpressionType::Void(ExpressionType::_Void);
		ExpressionType ExpressionType::Error(ExpressionType::_Error);
		CoreLib::Basic::String ExpressionType::ToString()
		{
			CoreLib::Basic::StringBuilder res;

			switch (BaseType)
			{
			case SimpleC::Compiler::ExpressionType::_Int:
				res.Append(L"int");
				break;
			case SimpleC::Compiler::ExpressionType::_Double:
				res.Append(L"double");
				break;
			case SimpleC::Compiler::ExpressionType::_Char:
				res.Append(L"char");
				break;
			case SimpleC::Compiler::ExpressionType::_String:
				res.Append(L"string");
				break;
			case SimpleC::Compiler::ExpressionType::_Function:
				res.Append(L"(");
				for (int i = 0; i < Func->Parameters.Count(); i++)
				{
					if (i > 0)
						res.Append(L",");
					res.Append(Func->Parameters[i]->Type->ToExpressionType().ToString());
				}
				res.Append(L") => ");
				res.Append(Func->ReturnType->ToExpressionType().ToString());
				break;
			case SimpleC::Compiler::ExpressionType::_Void:
				res.Append("void");
				break;
			default:
				break;
			}

			if (ArrayLength != 0)
			{
				res.Append(L"[");
				res.Append(CoreLib::Basic::String(ArrayLength));
				res.Append(L"]");
			}
			return res.ToString();
		}


		void ProgramSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitProgram(this);
		}
		void FunctionSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitFunction(this);
		}
		void BlockStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitBlockStatement(this);
		}
		void BreakStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitBreakStatement(this);
		}
		void ContinueStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitContinueStatement(this);
		}
		void DoWhileStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitDoWhileStatement(this);
		}
		void EmptyStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitEmptyStatement(this);
		}
		void ForStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitForStatement(this);
		}
		void IfStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitIfStatement(this);
		}
		void ReturnStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitReturnStatement(this);
		}
		void VarDeclrStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitVarDeclrStatement(this);
		}
		void VarDeclrStatementSyntaxNode::Variable::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitDeclrVariable(this);
		}
		void WhileStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitWhileStatement(this);
		}
		void ExpressionStatementSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitExpressionStatement(this);
		}
		void BinaryExpressionSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitBinaryExpression(this);
		}
		void ConstantExpressionSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitConstantExpression(this);
		}
		void IndexExpressionSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitIndexExpression(this);
		}
		void InvokeExpressionSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitInvokeExpression(this);
		}
		void UnaryExpressionSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitUnaryExpression(this);
		}
		void VarExpressionSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitVarExpression(this);
		}
		void ParameterSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitParameter(this);
		}
		void TypeSyntaxNode::Accept(SyntaxVisitor * visitor)
		{
			visitor->VisitType(this);
		}
	}
}