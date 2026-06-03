#include "SyntaxVisitors.h"
#include <iomanip>
#include <iostream>

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
			std::wcout << L" " << function->Name << L"(";
			for (auto & parameter : function->Parameters)
			{
				parameter->Accept(*this);
				std::wcout << L",";
			}
			std::wcout << L")\r\n";
			function->Body->Accept(*this);
		}

		void SyntaxPrinter::VisitBlockStatement(BlockStatementSyntaxNode & block_stmt_node)
		{
			auto * blockStmt = &block_stmt_node;
			std::wcout << L"{\r\n";
			for (auto & statement : blockStmt->Statements)
				statement->Accept(*this);
			std::wcout << L"}\r\n";
		}

		void SyntaxPrinter::VisitBreakStatement(BreakStatementSyntaxNode & stmt)
		{
			std::wcout << L"break;\r\n";
		}

		void SyntaxPrinter::VisitContinueStatement(ContinueStatementSyntaxNode & stmt)
		{
			std::wcout << L"continue;\r\n";
		}

		void SyntaxPrinter::VisitDoWhileStatement(DoWhileStatementSyntaxNode & stmt_node)
		{
			auto * stmt = &stmt_node;
			std::wcout << L"do\r\n";
			stmt->Statement->Accept(*this);
			std::wcout << L"while(";
			stmt->Predicate->Accept(*this);
			std::wcout << L");\r\n";
		}

		void SyntaxPrinter::VisitEmptyStatement(EmptyStatementSyntaxNode & stmt)
		{
			std::wcout << L";\r\n";
		}

		void SyntaxPrinter::VisitForStatement(ForStatementSyntaxNode & stmt_node)
		{
			auto * stmt = &stmt_node;
			std::wcout << L"for(";
			if (stmt->VarDeclr != nullptr)
				stmt->VarDeclr->Accept(*this);
			else
			{
				if(stmt->InitialExpression != nullptr)
					stmt->InitialExpression->Accept(*this);
				std::wcout << L";";
			}
			if (stmt->MarginExpression)
				stmt->MarginExpression->Accept(*this);
			std::wcout << L";";
			if (stmt->SideEffectExpression)
				stmt->SideEffectExpression->Accept(*this);
			std::wcout << L")\r\n";
			stmt->Statement->Accept(*this);
		}

		void SyntaxPrinter::VisitIfStatement(IfStatementSyntaxNode & stmt_node)
		{
			auto * stmt = &stmt_node;
			std::wcout << L"If(";
			stmt->Predicate->Accept(*this);
			std::wcout << L")\r\n";
			stmt->PositiveStatement->Accept(*this);
			if(stmt->NegativeStatement != nullptr)
			{
				std::wcout << L"else\r\n";
				stmt->NegativeStatement->Accept(*this);
			}
		}

		void SyntaxPrinter::VisitReturnStatement(ReturnStatementSyntaxNode & stmt_node)
		{
			auto * stmt = &stmt_node;
			std::wcout << L"return";
			if(stmt->Expression != nullptr)
			{
				std::wcout << L" ";
				stmt->Expression->Accept(*this);
				std::wcout << L";\r\n";
			}
		}

		void SyntaxPrinter::VisitVarDeclrStatement(VarDeclrStatementSyntaxNode & stmt_node)
		{
			auto * stmt = &stmt_node;
			stmt->Type->Accept(*this);
			std::wcout << L" ";
			for (auto & variable : stmt->Variables)
			{
				variable->Accept(*this);
				std::wcout << L",";
			}
			std::wcout << L";\r\n";
		}

		void SyntaxPrinter::VisitDeclrVariable(VarDeclrStatementSyntaxNode::Variable & variable_node)
		{
			auto * variable = &variable_node;
			std::wcout << variable->Name;
			if(variable->Expression != nullptr)
			{
				std::wcout << L" = ";
				variable->Expression->Accept(*this);
			}
		}

		void SyntaxPrinter::VisitWhileStatement(WhileStatementSyntaxNode & stmt_node)
		{
			auto * stmt = &stmt_node;
			std::wcout << L"while(";
			stmt->Predicate->Accept(*this);
			std::wcout << L")\r\n";
			stmt->Statement->Accept(*this);
		}

		void SyntaxPrinter::VisitExpressionStatement(ExpressionStatementSyntaxNode & stmt_node)
		{
			auto * stmt = &stmt_node;
			if (stmt->Expression)
				stmt->Expression->Accept(*this);
			std::wcout << L";\n";
		}

		void SyntaxPrinter::VisitBinaryExpression(BinaryExpressionSyntaxNode & expr_node)
		{
			auto * expr = &expr_node;
			std::wcout << L"(";
			expr->LeftExpression->Accept(*this);
			PrintOperator(expr->Operator);
			expr->RightExpression->Accept(*this);
			std::wcout << L")";
		}

		void SyntaxPrinter::VisitConstantExpression(ConstantExpressionSyntaxNode & expr_node)
		{
			auto * expr = &expr_node;
			switch (expr->ConstType)
			{
				case ConstantExpressionSyntaxNode::ConstantType::Int:
					std::wcout << expr->IntValue;
					break;
				case ConstantExpressionSyntaxNode::ConstantType::Double:
					std::wcout << std::fixed << std::setprecision(6) << expr->DoubleValue << std::defaultfloat;
					break;
				case ConstantExpressionSyntaxNode::ConstantType::Char:
					std::wcout << L"'" << expr->CharValue << L"'";
					break;
				case ConstantExpressionSyntaxNode::ConstantType::String:
					std::wcout << L"\"" << expr->StringValue << L"\"";
					break;
			default:
					std::wcout << L"ERROR";
				break;
			}
		}

		void SyntaxPrinter::VisitIndexExpression(IndexExpressionSyntaxNode & expr_node)
		{
			auto * expr = &expr_node;
			expr->BaseExpression->Accept(*this);
			std::wcout << L"[";
			expr->IndexExpression->Accept(*this);
			std::wcout << L"]";
		}

		void SyntaxPrinter::VisitInvokeExpression(InvokeExpressionSyntaxNode & expr_node)
		{
			auto * expr = &expr_node;
			expr->FunctionExpr->Accept(*this);
			std::wcout << L"(";
			for (auto & argument : expr->Arguments)
				argument->Accept(*this);
			std::wcout << L")";
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
			std::wcout << expr->Variable;
		}

		void SyntaxPrinter::VisitParameter(ParameterSyntaxNode & para_node)
		{
			auto * para = &para_node;
			para->Type->Accept(*this);
			std::wcout << L" " << para->Name;
		}

		void SyntaxPrinter::VisitType(TypeSyntaxNode & type_node)
		{
			auto * type = &type_node;
			std::wcout << type->TypeName;
			if(type->ArrayLength > 0)
				std::wcout << L"[" << type->ArrayLength << L"]";
		}

		void SyntaxPrinter::PrintOperator(Operator op)
		{
			switch (op)
			{
			case SimpleC::Compiler::Operator::Neg:
				std::wcout << L"-";
				break;
			case SimpleC::Compiler::Operator::Not:
				std::wcout << L"!";
				break;
			case SimpleC::Compiler::Operator::PreInc:
				std::wcout << L"++";
				break;
			case SimpleC::Compiler::Operator::PreDec:
				std::wcout << L"--";
				break;
			case SimpleC::Compiler::Operator::PostInc:
				std::wcout << L"++";
				break;
			case SimpleC::Compiler::Operator::PostDec:
				std::wcout << L"--";
				break;
			case SimpleC::Compiler::Operator::Mul:
				std::wcout << L"*";
				break;
			case SimpleC::Compiler::Operator::Div:
				std::wcout << L"/";
				break;
			case SimpleC::Compiler::Operator::Mod:
				std::wcout << L"%";
				break;
			case SimpleC::Compiler::Operator::Add:
				std::wcout << L"+";
				break;
			case SimpleC::Compiler::Operator::Sub:
				std::wcout << L"-";
				break;
			case SimpleC::Compiler::Operator::Lsh:
				std::wcout << L">>";
				break;
			case SimpleC::Compiler::Operator::Rsh:
				std::wcout << L"<<";
				break;
			case SimpleC::Compiler::Operator::Eql:
				std::wcout << L"==";
				break;
			case SimpleC::Compiler::Operator::Neq:
				std::wcout << L"!=";
				break;
			case SimpleC::Compiler::Operator::Greater:
				std::wcout << L">";
				break;
			case SimpleC::Compiler::Operator::Less:
				std::wcout << L"<";
				break;
			case SimpleC::Compiler::Operator::Geq:
				std::wcout << L">=";
				break;
			case SimpleC::Compiler::Operator::Leq:
				std::wcout << L"<=";
				break;
			case SimpleC::Compiler::Operator::BitAnd:
				std::wcout << L"&";
				break;
			case SimpleC::Compiler::Operator::BitXor:
				std::wcout << L"^";
				break;
			case SimpleC::Compiler::Operator::BitOr:
				std::wcout << L"|";
				break;
			case SimpleC::Compiler::Operator::And:
				std::wcout << L"&&";
				break;
			case SimpleC::Compiler::Operator::Or:
				std::wcout << L"||";
				break;
			case SimpleC::Compiler::Operator::Assign:
				std::wcout << L"=";
				break;
			default:
				std::wcout << L"ERROR";
				break;
			}
		}

		std::unique_ptr<SyntaxVisitor> CreateSyntaxPrinter()
		{
			return std::make_unique<SyntaxPrinter>();
		}
	}
}
