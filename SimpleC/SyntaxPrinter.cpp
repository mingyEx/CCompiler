#include "SyntaxVisitors.h"

namespace SimpleC
{
	namespace Compiler
	{
		class SyntaxPrinter : public SyntaxVisitor
		{
		public:
			virtual void VisitFunction(FunctionSyntaxNode *function);
			virtual void VisitBlockStatement(BlockStatementSyntaxNode *stmt);
			virtual void VisitBreakStatement(BreakStatementSyntaxNode *stmt);
			virtual void VisitContinueStatement(ContinueStatementSyntaxNode *stmt);
			virtual void VisitDoWhileStatement(DoWhileStatementSyntaxNode *stmt);
			virtual void VisitEmptyStatement(EmptyStatementSyntaxNode *stmt);
			virtual void VisitForStatement(ForStatementSyntaxNode *stmt);
			virtual void VisitIfStatement(IfStatementSyntaxNode *stmt);
			virtual void VisitReturnStatement(ReturnStatementSyntaxNode *stmt);
			virtual void VisitVarDeclrStatement(VarDeclrStatementSyntaxNode *stmt);
			virtual void VisitWhileStatement(WhileStatementSyntaxNode *stmt);
			virtual void VisitExpressionStatement(ExpressionStatementSyntaxNode *stmt);
			virtual void VisitBinaryExpression(BinaryExpressionSyntaxNode *expr);
			virtual void VisitConstantExpression(ConstantExpressionSyntaxNode *expr);
			virtual void VisitIndexExpression(IndexExpressionSyntaxNode *expr);
			virtual void VisitInvokeExpression(InvokeExpressionSyntaxNode *expr);
			virtual void VisitUnaryExpression(UnaryExpressionSyntaxNode *expr);
			virtual void VisitVarExpression(VarExpressionSyntaxNode *expr);
			virtual void VisitParameter(ParameterSyntaxNode *para);
			virtual void VisitType(TypeSyntaxNode *type);
			virtual void VisitDeclrVariable(VarDeclrStatementSyntaxNode::Variable * variable);

			void PrintOperator(Operator op);
		};

		void SyntaxPrinter::VisitFunction(FunctionSyntaxNode *function)
		{
			function->ReturnType->Accept(this);
			wprintf_s(L" %s(", function->Name.Buffer());
			function->Parameters.ForEach([&](RefPtr<ParameterSyntaxNode> para){para->Accept(this); printf(",");});
			printf(")\r\n");
			function->Body->Accept(this);
		}

		void SyntaxPrinter::VisitBlockStatement(BlockStatementSyntaxNode *blockStmt)
		{
			printf("{\r\n");
			blockStmt->Statements.ForEach([&](RefPtr<StatementSyntaxNode> stmt){stmt->Accept(this);});
			printf("}\r\n");
		}

		void SyntaxPrinter::VisitBreakStatement(BreakStatementSyntaxNode *stmt)
		{
			printf("break;\r\n");
		}

		void SyntaxPrinter::VisitContinueStatement(ContinueStatementSyntaxNode *stmt)
		{
			printf("continue;\r\n");
		}

		void SyntaxPrinter::VisitDoWhileStatement(DoWhileStatementSyntaxNode *stmt)
		{
			printf("do\r\n");
			stmt->Statement->Accept(this);
			printf("while(");
			stmt->Predicate->Accept(this);
			printf(");\r\n");
		}

		void SyntaxPrinter::VisitEmptyStatement(EmptyStatementSyntaxNode *stmt)
		{
			printf(";\r\n");
		}

		void SyntaxPrinter::VisitForStatement(ForStatementSyntaxNode *stmt)
		{
			printf("for(");
			if (stmt->VarDeclr != NULL)
				stmt->VarDeclr->Accept(this);
			else
			{
				if(stmt->InitialExpression != NULL)
					stmt->InitialExpression->Accept(this);
				printf(";");
			}
			if (stmt->MarginExpression)
				stmt->MarginExpression->Accept(this);
			printf(";");
			if (stmt->SideEffectExpression)
				stmt->SideEffectExpression->Accept(this);
			printf(")\r\n");
			stmt->Statement->Accept(this);
		}

		void SyntaxPrinter::VisitIfStatement(IfStatementSyntaxNode *stmt)
		{
			printf("If(");
			stmt->Predicate->Accept(this);
			printf(")\r\n");
			stmt->PositiveStatement->Accept(this);
			if(stmt->NegativeStatement != NULL)
			{
				printf("else\r\n");
				stmt->NegativeStatement->Accept(this);
			}
		}

		void SyntaxPrinter::VisitReturnStatement(ReturnStatementSyntaxNode *stmt)
		{
			printf("return");
			if(stmt->Expression != NULL)
			{
				printf(" ");
				stmt->Expression->Accept(this);
				printf(";\r\n");
			}
		}

		void SyntaxPrinter::VisitVarDeclrStatement(VarDeclrStatementSyntaxNode *stmt)
		{
			stmt->Type->Accept(this);
			printf(" ");
			stmt->Variables.ForEach([&](RefPtr<VarDeclrStatementSyntaxNode::Variable> var){var->Accept(this); printf(",");});
			printf(";\r\n");
		}

		void SyntaxPrinter::VisitDeclrVariable(VarDeclrStatementSyntaxNode::Variable * variable)
		{
			wprintf_s(L"%s", variable->Name.Buffer());
			if(variable->Expression != NULL)
			{
				printf(" = ");
				variable->Expression->Accept(this);
			}
		}

		void SyntaxPrinter::VisitWhileStatement(WhileStatementSyntaxNode *stmt)
		{
			printf("while(");
			stmt->Predicate->Accept(this);
			printf(")\r\n");
			stmt->Statement->Accept(this);
		}

		void SyntaxPrinter::VisitExpressionStatement(ExpressionStatementSyntaxNode *stmt)
		{
			if (stmt->Expression)
				stmt->Expression->Accept(this);
			printf(";\n");
		}

		void SyntaxPrinter::VisitBinaryExpression(BinaryExpressionSyntaxNode *expr)
		{
			printf("(");
			expr->LeftExpression->Accept(this);
			PrintOperator(expr->Operator);
			expr->RightExpression->Accept(this);
			printf(")");
		}

		void SyntaxPrinter::VisitConstantExpression(ConstantExpressionSyntaxNode *expr)
		{
			switch (expr->ConstType)
			{
				case ConstantExpressionSyntaxNode::ConstantType::Int:
					printf("%d", expr->IntValue);
					break;
				case ConstantExpressionSyntaxNode::ConstantType::Double:
					printf("%f", expr->DoubleValue);
					break;
				case ConstantExpressionSyntaxNode::ConstantType::Char:
					wprintf_s(L"'%c'", expr->CharValue);
					break;
				case ConstantExpressionSyntaxNode::ConstantType::String:
					wprintf_s(L"\"%s\"", expr->StringValue.Buffer());
					break;
			default:
					printf("ERROR");
				break;
			}
		}

		void SyntaxPrinter::VisitIndexExpression(IndexExpressionSyntaxNode *expr)
		{
			expr->BaseExpression->Accept(this);
			printf("[");
			expr->IndexExpression->Accept(this);
			printf("]");
		}

		void SyntaxPrinter::VisitInvokeExpression(InvokeExpressionSyntaxNode *expr)
		{
			expr->FunctionExpr->Accept(this);
			printf("(");
			expr->Arguments.ForEach([&](RefPtr<ExpressionSyntaxNode> exp){exp->Accept(this);});
			printf(")");
		}

		void SyntaxPrinter::VisitUnaryExpression(UnaryExpressionSyntaxNode *expr)
		{
			if (expr->Operator == Operator::PostDec || expr ->Operator == Operator::PostInc)
			{
				expr->Expression->Accept(this);
				PrintOperator(expr->Operator);
			}
			else
			{
				PrintOperator(expr->Operator);
				expr->Expression->Accept(this);
			}
		}

		void SyntaxPrinter::VisitVarExpression(VarExpressionSyntaxNode *expr)
		{
			wprintf_s(L"%s", expr->Variable.Buffer());
			//printf("%s", expr->Variable.ToMultiByteString());
		}

		void SyntaxPrinter::VisitParameter(ParameterSyntaxNode *para)
		{
			para->Type->Accept(this);
			printf(" ");
			wprintf_s(L"%s", para->Name.Buffer());
		}

		void SyntaxPrinter::VisitType(TypeSyntaxNode *type)
		{
			wprintf_s(L"%s", type->TypeName.Buffer());
			if(type->ArrayLength > 0)
				printf("[%d]", type->ArrayLength);
		}

		void SyntaxPrinter::PrintOperator(Operator op)
		{
			switch (op)
			{
			case SimpleC::Compiler::Operator::Neg:
				printf("-");
				break;
			case SimpleC::Compiler::Operator::Not:
				printf("!");
				break;
			case SimpleC::Compiler::Operator::PreInc:
				printf("++");
				break;
			case SimpleC::Compiler::Operator::PreDec:
				printf("--");
				break;
			case SimpleC::Compiler::Operator::PostInc:
				printf("++");
				break;
			case SimpleC::Compiler::Operator::PostDec:
				printf("--");
				break;
			case SimpleC::Compiler::Operator::Mul:
				printf("*");
				break;
			case SimpleC::Compiler::Operator::Div:
				printf("/");
				break;
			case SimpleC::Compiler::Operator::Mod:
				printf("%");
				break;
			case SimpleC::Compiler::Operator::Add:
				printf("+");
				break;
			case SimpleC::Compiler::Operator::Sub:
				printf("-");
				break;
			case SimpleC::Compiler::Operator::Lsh:
				printf(">>");
				break;
			case SimpleC::Compiler::Operator::Rsh:
				printf("<<");
				break;
			case SimpleC::Compiler::Operator::Eql:
				printf("==");
				break;
			case SimpleC::Compiler::Operator::Neq:
				printf("!=");
				break;
			case SimpleC::Compiler::Operator::Greater:
				printf(">");
				break;
			case SimpleC::Compiler::Operator::Less:
				printf("<");
				break;
			case SimpleC::Compiler::Operator::Geq:
				printf(">=");
				break;
			case SimpleC::Compiler::Operator::Leq:
				printf("<=");
				break;
			case SimpleC::Compiler::Operator::BitAnd:
				printf("&");
				break;
			case SimpleC::Compiler::Operator::BitXor:
				printf("^");
				break;
			case SimpleC::Compiler::Operator::BitOr:
				printf("|");
				break;
			case SimpleC::Compiler::Operator::And:
				printf("&&");
				break;
			case SimpleC::Compiler::Operator::Or:
				printf("||");
				break;
			case SimpleC::Compiler::Operator::Assign:
				printf("=");
				break;
			default:
				printf("ERROR");
				break;
			}
		}

		SyntaxVisitor * CreateSyntaxPrinter()
		{
			return new SyntaxPrinter();
		}
	}
}