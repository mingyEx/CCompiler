#include "SyntaxVisitors.h"

namespace SimpleC
{
	namespace Compiler
	{
		class SyntaxPrinter : public SyntaxVisitor
		{
		public:
			virtual void VisitFunction(FunctionSyntaxNode & function);
			virtual void VisitBlockStatement(BlockStatementSyntaxNode & stmt);
			virtual void VisitBreakStatement(BreakStatementSyntaxNode & stmt);
			virtual void VisitContinueStatement(ContinueStatementSyntaxNode & stmt);
			virtual void VisitDoWhileStatement(DoWhileStatementSyntaxNode & stmt);
			virtual void VisitEmptyStatement(EmptyStatementSyntaxNode & stmt);
			virtual void VisitForStatement(ForStatementSyntaxNode & stmt);
			virtual void VisitIfStatement(IfStatementSyntaxNode & stmt);
			virtual void VisitReturnStatement(ReturnStatementSyntaxNode & stmt);
			virtual void VisitVarDeclrStatement(VarDeclrStatementSyntaxNode & stmt);
			virtual void VisitWhileStatement(WhileStatementSyntaxNode & stmt);
			virtual void VisitExpressionStatement(ExpressionStatementSyntaxNode & stmt);
			virtual void VisitBinaryExpression(BinaryExpressionSyntaxNode & expr);
			virtual void VisitConstantExpression(ConstantExpressionSyntaxNode & expr);
			virtual void VisitIndexExpression(IndexExpressionSyntaxNode & expr);
			virtual void VisitInvokeExpression(InvokeExpressionSyntaxNode & expr);
			virtual void VisitUnaryExpression(UnaryExpressionSyntaxNode & expr);
			virtual void VisitVarExpression(VarExpressionSyntaxNode & expr);
			virtual void VisitParameter(ParameterSyntaxNode & para);
			virtual void VisitType(TypeSyntaxNode & type);
			virtual void VisitDeclrVariable(VarDeclrStatementSyntaxNode::Variable & variable);

			void PrintOperator(Operator op);
		};

		void SyntaxPrinter::VisitFunction(FunctionSyntaxNode & function_node)
		{
			auto * function = &function_node;
			function->ReturnType->Accept(*this);
			wprintf_s(L" %s(", function->Name.c_str());
			for (auto & parameter : function->Parameters)
			{
				parameter->Accept(*this);
				printf(",");
			}
			printf(")\r\n");
			function->Body->Accept(*this);
		}

		void SyntaxPrinter::VisitBlockStatement(BlockStatementSyntaxNode & block_stmt_node)
		{
			auto * blockStmt = &block_stmt_node;
			printf("{\r\n");
			for (auto & statement : blockStmt->Statements)
				statement->Accept(*this);
			printf("}\r\n");
		}

		void SyntaxPrinter::VisitBreakStatement(BreakStatementSyntaxNode & stmt)
		{
			printf("break;\r\n");
		}

		void SyntaxPrinter::VisitContinueStatement(ContinueStatementSyntaxNode & stmt)
		{
			printf("continue;\r\n");
		}

		void SyntaxPrinter::VisitDoWhileStatement(DoWhileStatementSyntaxNode & stmt_node)
		{
			auto * stmt = &stmt_node;
			printf("do\r\n");
			stmt->Statement->Accept(*this);
			printf("while(");
			stmt->Predicate->Accept(*this);
			printf(");\r\n");
		}

		void SyntaxPrinter::VisitEmptyStatement(EmptyStatementSyntaxNode & stmt)
		{
			printf(";\r\n");
		}

		void SyntaxPrinter::VisitForStatement(ForStatementSyntaxNode & stmt_node)
		{
			auto * stmt = &stmt_node;
			printf("for(");
			if (stmt->VarDeclr != NULL)
				stmt->VarDeclr->Accept(*this);
			else
			{
				if(stmt->InitialExpression != NULL)
					stmt->InitialExpression->Accept(*this);
				printf(";");
			}
			if (stmt->MarginExpression)
				stmt->MarginExpression->Accept(*this);
			printf(";");
			if (stmt->SideEffectExpression)
				stmt->SideEffectExpression->Accept(*this);
			printf(")\r\n");
			stmt->Statement->Accept(*this);
		}

		void SyntaxPrinter::VisitIfStatement(IfStatementSyntaxNode & stmt_node)
		{
			auto * stmt = &stmt_node;
			printf("If(");
			stmt->Predicate->Accept(*this);
			printf(")\r\n");
			stmt->PositiveStatement->Accept(*this);
			if(stmt->NegativeStatement != NULL)
			{
				printf("else\r\n");
				stmt->NegativeStatement->Accept(*this);
			}
		}

		void SyntaxPrinter::VisitReturnStatement(ReturnStatementSyntaxNode & stmt_node)
		{
			auto * stmt = &stmt_node;
			printf("return");
			if(stmt->Expression != NULL)
			{
				printf(" ");
				stmt->Expression->Accept(*this);
				printf(";\r\n");
			}
		}

		void SyntaxPrinter::VisitVarDeclrStatement(VarDeclrStatementSyntaxNode & stmt_node)
		{
			auto * stmt = &stmt_node;
			stmt->Type->Accept(*this);
			printf(" ");
			for (auto & variable : stmt->Variables)
			{
				variable->Accept(*this);
				printf(",");
			}
			printf(";\r\n");
		}

		void SyntaxPrinter::VisitDeclrVariable(VarDeclrStatementSyntaxNode::Variable & variable_node)
		{
			auto * variable = &variable_node;
			wprintf_s(L"%s", variable->Name.c_str());
			if(variable->Expression != NULL)
			{
				printf(" = ");
				variable->Expression->Accept(*this);
			}
		}

		void SyntaxPrinter::VisitWhileStatement(WhileStatementSyntaxNode & stmt_node)
		{
			auto * stmt = &stmt_node;
			printf("while(");
			stmt->Predicate->Accept(*this);
			printf(")\r\n");
			stmt->Statement->Accept(*this);
		}

		void SyntaxPrinter::VisitExpressionStatement(ExpressionStatementSyntaxNode & stmt_node)
		{
			auto * stmt = &stmt_node;
			if (stmt->Expression)
				stmt->Expression->Accept(*this);
			printf(";\n");
		}

		void SyntaxPrinter::VisitBinaryExpression(BinaryExpressionSyntaxNode & expr_node)
		{
			auto * expr = &expr_node;
			printf("(");
			expr->LeftExpression->Accept(*this);
			PrintOperator(expr->Operator);
			expr->RightExpression->Accept(*this);
			printf(")");
		}

		void SyntaxPrinter::VisitConstantExpression(ConstantExpressionSyntaxNode & expr_node)
		{
			auto * expr = &expr_node;
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
					wprintf_s(L"\"%s\"", expr->StringValue.c_str());
					break;
			default:
					printf("ERROR");
				break;
			}
		}

		void SyntaxPrinter::VisitIndexExpression(IndexExpressionSyntaxNode & expr_node)
		{
			auto * expr = &expr_node;
			expr->BaseExpression->Accept(*this);
			printf("[");
			expr->IndexExpression->Accept(*this);
			printf("]");
		}

		void SyntaxPrinter::VisitInvokeExpression(InvokeExpressionSyntaxNode & expr_node)
		{
			auto * expr = &expr_node;
			expr->FunctionExpr->Accept(*this);
			printf("(");
			for (auto & argument : expr->Arguments)
				argument->Accept(*this);
			printf(")");
		}

		void SyntaxPrinter::VisitUnaryExpression(UnaryExpressionSyntaxNode & expr_node)
		{
			auto * expr = &expr_node;
			if (expr->Operator == Operator::PostDec || expr ->Operator == Operator::PostInc)
			{
				expr->Expression->Accept(*this);
				PrintOperator(expr->Operator);
			}
			else
			{
				PrintOperator(expr->Operator);
				expr->Expression->Accept(*this);
			}
		}

		void SyntaxPrinter::VisitVarExpression(VarExpressionSyntaxNode & expr_node)
		{
			auto * expr = &expr_node;
			wprintf_s(L"%s", expr->Variable.c_str());
			//printf("%s", expr->Variable.ToMultiByteString());
		}

		void SyntaxPrinter::VisitParameter(ParameterSyntaxNode & para_node)
		{
			auto * para = &para_node;
			para->Type->Accept(*this);
			printf(" ");
			wprintf_s(L"%s", para->Name.c_str());
		}

		void SyntaxPrinter::VisitType(TypeSyntaxNode & type_node)
		{
			auto * type = &type_node;
			wprintf_s(L"%s", type->TypeName.c_str());
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
				printf("%%");
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

		std::unique_ptr<SyntaxVisitor> CreateSyntaxPrinter()
		{
			return std::make_unique<SyntaxPrinter>();
		}
	}
}
