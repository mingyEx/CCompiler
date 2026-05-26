#include "Syntax.h"
#include "SyntaxVisitors.h"
#include <string>

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
		std::wstring ExpressionType::ToString()
		{
			std::wstring res;

			switch (BaseType)
			{
			case SimpleC::Compiler::ExpressionType::_Int:
				res += L"int";
				break;
			case SimpleC::Compiler::ExpressionType::_Double:
				res += L"double";
				break;
			case SimpleC::Compiler::ExpressionType::_Char:
				res += L"char";
				break;
			case SimpleC::Compiler::ExpressionType::_String:
				res += L"string";
				break;
			case SimpleC::Compiler::ExpressionType::_Function:
				res += L"(";
				for (size_t i = 0; i < Func->Parameters.size(); i++)
				{
					if (i > 0)
						res += L",";
					res += Func->Parameters[i]->Type->ToExpressionType().ToString();
				}
				res += L") => ";
				res += Func->ReturnType->ToExpressionType().ToString();
				break;
			case SimpleC::Compiler::ExpressionType::_Void:
				res += L"void";
				break;
			default:
				break;
			}

			if (ArrayLength != 0)
			{
				res += L"[";
				res += std::to_wstring(ArrayLength);
				res += L"]";
			}
			return res;
		}


		void ProgramSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitProgram(*this);
		}
		void FunctionSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitFunction(*this);
		}
		void BlockStatementSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitBlockStatement(*this);
		}
		void BreakStatementSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitBreakStatement(*this);
		}
		void ContinueStatementSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitContinueStatement(*this);
		}
		void DoWhileStatementSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitDoWhileStatement(*this);
		}
		void EmptyStatementSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitEmptyStatement(*this);
		}
		void ForStatementSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitForStatement(*this);
		}
		void IfStatementSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitIfStatement(*this);
		}
		void ReturnStatementSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitReturnStatement(*this);
		}
		void VarDeclrStatementSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitVarDeclrStatement(*this);
		}
		void VarDeclrStatementSyntaxNode::Variable::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitDeclrVariable(*this);
		}
		void WhileStatementSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitWhileStatement(*this);
		}
		void ExpressionStatementSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitExpressionStatement(*this);
		}
		void BinaryExpressionSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitBinaryExpression(*this);
		}
		void ConstantExpressionSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitConstantExpression(*this);
		}
		void IndexExpressionSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitIndexExpression(*this);
		}
		void InvokeExpressionSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitInvokeExpression(*this);
		}
		void UnaryExpressionSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitUnaryExpression(*this);
		}
		void VarExpressionSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitVarExpression(*this);
		}
		void ParameterSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitParameter(*this);
		}
		void TypeSyntaxNode::Accept(SyntaxVisitor & visitor)
		{
			visitor.VisitType(*this);
		}
	}
}
