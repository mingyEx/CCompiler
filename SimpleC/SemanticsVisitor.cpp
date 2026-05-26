#include "SyntaxVisitors.h"
#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include "IL/CompileError.h"

namespace SimpleC
{
	namespace Compiler
	{
		class SemanticsVisitor : public SyntaxVisitor
		{
			ProgramSyntaxNode * program;
			FunctionSyntaxNode * function;		
			std::vector<SyntaxNode *> loops;	// loop nesting stack used to validate break/continue
			std::vector<CompileError> & errors;

			void Error(int id, std::wstring text, SyntaxNode * node)
			{
				errors.push_back(CompileError(std::move(text), node->FileName, id, node->Line, node->Col));
			}
		public:
			SemanticsVisitor(std::vector<CompileError> & _errors)
				:errors(_errors)
			{}
			//Check if function names are duplicated
			virtual void VisitProgram(ProgramSyntaxNode & program_node)
			{
				auto * program = &program_node;
				std::set<std::wstring> funcNames;
				this->program = program;
				for (auto & func : program->Functions)
				{
					if (funcNames.find(func->Name) != funcNames.end())
						Error(30001, L"Function \'" + func->Name + L"\' redefinition.", func.get());
					else
						funcNames.insert(func->Name);
					func->Accept(*this);
				}
			}

			//
			virtual void VisitFunction(FunctionSyntaxNode & function_node) override 
			{
				auto * function = &function_node;
				this->function = function;
				//Check if there is a return type of the form void []
				const auto & returnType = function->ReturnType->ToExpressionType();
				if(returnType.BaseType == ExpressionType::_Void && returnType.IsArray)
					Error(30024, L"Function return type can not be 'void' array.", function->ReturnType.get());
				
				//Detect if Parameter is duplicated
				std::set<std::wstring> paraNames;
				for (auto & para : function->Parameters)
				{
					if (paraNames.find(para->Name) != paraNames.end())
						Error(30002, L"Parameter \'" + para->Name + L"\' already defined.", para.get());
					else
						paraNames.insert(para->Name);
					if(para->Type->ToExpressionType().BaseType == ExpressionType::_Void)
						Error(30016, L"'void' can not be parameter type.", para.get());
					para->Accept(*this);
				}
				function->Body->Accept(*this);
				this->function = nullptr;
			}
			//class Lambda01
			//{
			//public:
			//	SemanticsVisitor* _this;
			//	int& cdf;
			//	Lambda01(int& _cdf, SemanticsVisitor* _this)
			//		:cdf(_cdf), _this(_this)
			//	{}
			//	void operator ()(std::shared_ptr<StatementSyntaxNode>& node)
			//	{
			//		int xx = cdf;
			//		node->Accept(_this);
			//	}
			//};
			virtual void VisitBlockStatement(BlockStatementSyntaxNode & stmt_node)
			{
				auto * stmt = &stmt_node;
				//int cdf = 50;
				//Lambda01 lambda(cdf, this);
				//stmt->Statements.ForEach(lambda);
				for (auto & node : stmt->Statements)
					node->Accept(*this);
			}

			virtual void VisitBreakStatement(BreakStatementSyntaxNode & stmt_node)
			{
				auto * stmt = &stmt_node;
				if (loops.empty())
					Error(30003, L"'break' must appear inside loop constructs.", stmt);
			}
			virtual void VisitContinueStatement(ContinueStatementSyntaxNode & stmt_node)
			{
				auto * stmt = &stmt_node;
				if (loops.empty())
					Error(30004, L"'continue' must appear inside loop constructs.", stmt);
			}

			virtual void VisitDoWhileStatement(DoWhileStatementSyntaxNode & stmt_node)	
				// Enter loop scope before checking the predicate and body.
			{
				auto * stmt = &stmt_node;
				loops.push_back(stmt);
				if(stmt->Predicate != nullptr)
					stmt->Predicate->Accept(*this);
				if(stmt->Predicate->Type != ExpressionType::Error && stmt->Predicate->Type != ExpressionType::Int)
					Error(30005, L"'while': expression must evaluate to int.", stmt);
				stmt->Statement->Accept(*this);	
				loops.pop_back();
			}
			virtual void VisitEmptyStatement(EmptyStatementSyntaxNode & stmt){}
			virtual void VisitForStatement(ForStatementSyntaxNode & stmt_node)
			{
				auto * stmt = &stmt_node;
				loops.push_back(stmt);

				if (stmt->VarDeclr != nullptr)		
					stmt->VarDeclr->Accept(*this);

				if (stmt->InitialExpression != nullptr)
					stmt->InitialExpression->Accept(*this);

				if (stmt->MarginExpression != nullptr)
					stmt->MarginExpression->Accept(*this);

				if (stmt->SideEffectExpression != nullptr)
					stmt->SideEffectExpression->Accept(*this);

				stmt->Statement->Accept(*this);

				loops.pop_back();
			}
			virtual void VisitIfStatement(IfStatementSyntaxNode & stmt_node)
			{
				auto * stmt = &stmt_node;
				if (stmt->Predicate != nullptr)
					stmt->Predicate->Accept(*this);
				if (stmt->Predicate->Type != ExpressionType::Error && stmt->Predicate->Type != ExpressionType::Int)
					Error(30006, L"'if': expression must evaluate to int.", stmt);

				if (stmt->PositiveStatement != nullptr)
					stmt->PositiveStatement->Accept(*this);
				
				if (stmt->NegativeStatement != nullptr)
					stmt->NegativeStatement->Accept(*this);
			}
			virtual void VisitReturnStatement(ReturnStatementSyntaxNode & stmt_node)
			{
				auto * stmt = &stmt_node;
				if (stmt->Expression == nullptr)
				{
					if(function->ReturnType->ToExpressionType() != ExpressionType::Void)
						Error(30006, L"'return' should have an expression.", stmt);
				}
				else
				{
					stmt->Expression->Accept(*this);
					if(stmt->Expression->Type != ExpressionType::Error && stmt->Expression->Type != function->ReturnType->ToExpressionType())
						Error(30007, L"Expression type '" + stmt->Expression->Type.ToString() + L"' does not match function's return type '" + function->ReturnType->ToExpressionType().ToString() + L"'", stmt);
				}
			}
			virtual void VisitVarDeclrStatement(VarDeclrStatementSyntaxNode & stmt_node)
			{
				auto * stmt = &stmt_node;
				for (auto & para : stmt->Variables)
				{
					VariableDeclr varDeclr;
					varDeclr.Name = para->Name;
					const auto existing = std::find(function->Variables.begin(), function->Variables.end(), varDeclr);
					if(existing != function->Variables.end())
						Error(30008, L"Variable " + para->Name + L" already defined.", para.get());
					
					varDeclr.Type = stmt->Type->ToExpressionType();
					if(varDeclr.Type.BaseType == ExpressionType::_Void)
						Error(30009, L"Invalid type 'void'.", stmt);
					if(varDeclr.Type.IsArray && varDeclr.Type.ArrayLength <= 0)
						Error(30025, L"Array size must larger than zero.", stmt);

					function->Variables.push_back(varDeclr);
					if(para->Expression != nullptr)
						para->Expression->Accept(*this);
				}
			}
			virtual void VisitWhileStatement(WhileStatementSyntaxNode & stmt_node)
			{
				auto * stmt = &stmt_node;
				loops.push_back(stmt);
				stmt->Predicate->Accept(*this);
				if (stmt->Predicate->Type != ExpressionType::Error && stmt->Predicate->Type != ExpressionType::Int)
					Error(30010, L"'while': expression must evaluate to int.", stmt);
				stmt->Statement->Accept(*this);
				loops.pop_back();
			}
			virtual void VisitExpressionStatement(ExpressionStatementSyntaxNode & stmt_node)
			{
				auto * stmt = &stmt_node;
				stmt->Expression->Accept(*this);
			}
			virtual void VisitBinaryExpression(BinaryExpressionSyntaxNode & expr_node)
			{
				auto * expr = &expr_node;
				// Visit both sides first so their inferred expression types are available below.
				expr->LeftExpression->Accept(*this);
				expr->RightExpression->Accept(*this);
				auto & leftType = expr->LeftExpression->Type;
				auto & rightType = expr->RightExpression->Type;
				// Then compute the result type from the operator and operand types.
				switch (expr->Operator)
				{
				case Operator::Add:
					if (leftType == ExpressionType::Int && rightType == ExpressionType::Int)
						expr->Type = ExpressionType::Int;
					else if (leftType == ExpressionType::Int && rightType == ExpressionType::Double)
						expr->Type = ExpressionType::Double;
					else if (leftType == ExpressionType::Double && rightType == ExpressionType::Int)
						expr->Type = ExpressionType::Double;
					else if (leftType == ExpressionType::Double && rightType == ExpressionType::Double)
						expr->Type = ExpressionType::Double;
					else if (leftType == ExpressionType::Char && rightType == ExpressionType::Char)
						expr->Type = ExpressionType::String;
					else if (leftType == ExpressionType::Char && rightType == ExpressionType::String)
						expr->Type = ExpressionType::String;
					else if (leftType == ExpressionType::String && rightType == ExpressionType::Char)
						expr->Type = ExpressionType::String;
					else if (leftType == ExpressionType::String && rightType == ExpressionType::String)
						expr->Type = ExpressionType::String;
					else
						expr->Type = ExpressionType::Error;
					break;
				case Operator::Sub:
				case Operator::Mul:
				case Operator::Div:
					if (leftType == ExpressionType::Int && rightType == ExpressionType::Int)
						expr->Type = ExpressionType::Int;
					else if (leftType == ExpressionType::Int && rightType == ExpressionType::Double)
						expr->Type = ExpressionType::Double;
					else if (leftType == ExpressionType::Double && rightType == ExpressionType::Int)
						expr->Type = ExpressionType::Double;
					else if (leftType == ExpressionType::Double && rightType == ExpressionType::Double)
						expr->Type = ExpressionType::Double;
					else
						expr->Type = ExpressionType::Error;
					break;
				case Operator::Mod:
				case Operator::Rsh:
				case Operator::Lsh:
				case Operator::BitAnd:
				case Operator::BitOr:
				case Operator::BitXor:
				case Operator::And:
				case Operator::Or:
					if (leftType == ExpressionType::Int && rightType == ExpressionType::Int)
						expr->Type = ExpressionType::Int;
					else
						expr->Type = ExpressionType::Error;
					break;
				case Operator::Neq:
				case Operator::Eql:
				case Operator::Greater:
				case Operator::Geq:
				case Operator::Less:
				case Operator::Leq:
					if (leftType == ExpressionType::Int && rightType == ExpressionType::Int)
						expr->Type = ExpressionType::Int;
					else if (leftType == ExpressionType::Int && rightType == ExpressionType::Double)
						expr->Type = ExpressionType::Int;
					else if (leftType == ExpressionType::Double && rightType == ExpressionType::Int)
						expr->Type = ExpressionType::Int;
					else if (leftType == ExpressionType::Double && rightType == ExpressionType::Double)
						expr->Type = ExpressionType::Int;
					else if (leftType == ExpressionType::Char && rightType == ExpressionType::Char)
						expr->Type = ExpressionType::Int;
					else if (leftType == ExpressionType::String && rightType == ExpressionType::String)
						expr->Type = ExpressionType::Int;
					else
						expr->Type = ExpressionType::Error;
					break;
				case Operator::Assign:
					if (!leftType.IsLeftValue && leftType != ExpressionType::Error)	
						Error(30011, L"Only variables or array elements can be assigned a value.", expr->LeftExpression.get());
					expr->LeftExpression->Access = ExpressionAccess::Write;
					if (leftType == rightType)
						expr->Type = leftType;
					else if (leftType == ExpressionType::Double && rightType == ExpressionType::Int)
						expr->Type = ExpressionType::Double;
					else
						expr->Type = ExpressionType::Error;
					break;
				default:
						expr->Type = ExpressionType::Error;
					break;
				}
				
				if (expr->Type == ExpressionType::Error &&
					leftType != ExpressionType::Error && rightType != ExpressionType::Error)
					Error(30012, L"Type mismatch: '" + leftType.ToString() + L"' and '" + rightType.ToString() + L"'.", expr);
			}
			virtual void VisitConstantExpression(ConstantExpressionSyntaxNode & expr_node)
			{
				auto * expr = &expr_node;
				switch (expr->ConstType)
				{
				case ConstantExpressionSyntaxNode::ConstantType::Int:
					expr->Type = ExpressionType::Int;
					break;
				case ConstantExpressionSyntaxNode::ConstantType::Double:
					expr->Type = ExpressionType::Double;
					break;
				case ConstantExpressionSyntaxNode::ConstantType::Char:
					expr->Type = ExpressionType::Char;
					break;
				case ConstantExpressionSyntaxNode::ConstantType::String:
					expr->Type = ExpressionType::String;
					break;
				default:
					expr->Type = ExpressionType::Error;
					throw "Invalid constant type.";
					break;
				}
			}
			virtual void VisitIndexExpression(IndexExpressionSyntaxNode & expr_node)
			{
				auto * expr = &expr_node;
				expr->BaseExpression->Accept(*this);
				expr->IndexExpression->Accept(*this);
				if (expr->BaseExpression->Type == ExpressionType::Error)
					expr->Type = ExpressionType::Error;
				else
				{
					if (expr->BaseExpression->Type != ExpressionType::String &&
						expr->BaseExpression->Type.ArrayLength == 0)
					{
						Error(30013, L"'[]' can only index on arrays and strings.", expr);
						expr->Type = ExpressionType::Error;
					}
					if (expr->IndexExpression->Type != ExpressionType::Int)
					{
						Error(30014, L"Index expression must evaluate to int.", expr);
						expr->Type = ExpressionType::Error;
					}
				}
				if (expr->BaseExpression->Type == ExpressionType::String)
					expr->Type = ExpressionType::Char;
				else if (expr->BaseExpression->Type.ArrayLength > 0)
					expr->Type.BaseType = expr->BaseExpression->Type.BaseType;
				expr->Type.IsLeftValue = true;
				expr->Type.IsReference = true;
			}
			virtual void VisitInvokeExpression(InvokeExpressionSyntaxNode & expr_node)
			{
				auto * expr = &expr_node;
				expr->FunctionExpr->Accept(*this);
				if (expr->FunctionExpr->Type.BaseType == ExpressionType::_Function &&
					expr->FunctionExpr->Type.ArrayLength == 0)
				{
					auto & func = *(expr->FunctionExpr->Type.Func);
					expr->Type = func.ReturnType->ToExpressionType();
					if(expr->Arguments.size() != func.Parameters.size())
						Error(30017, L"Function '" + func.Name + L"' does not take " + std::to_wstring(expr->Arguments.size()) + L"arguments.", expr);
					else
						for(size_t i = 0; i < expr->Arguments.size(); i++)
						{
							expr->Arguments[i]->Accept(*this);
							auto paraType = func.Parameters[i]->Type->ToExpressionType();
							if(expr->Arguments[i]->Type != ExpressionType::Error &&
								expr->Arguments[i]->Type != paraType)
							{
								if(!(expr->Arguments[i]->Type == ExpressionType::Int && paraType == ExpressionType::Double))
									Error(30018, L"Argument " + std::to_wstring(i + 1) + L" does not evaluate to parameter type '" + paraType.ToString() + L"'", expr);
							}
						}
				}
				else
				{
					if (expr->FunctionExpr->Type != ExpressionType::Error)
						Error(30015, L"Expression preceeding parenthesis of apparent function call must have function type.", expr);
					expr->Type = ExpressionType::Error;
				}
			}

			std::wstring OperatorToString(Operator op)
			{
				switch (op)
				{
				case SimpleC::Compiler::Operator::Neg:
					return L"-";
				case SimpleC::Compiler::Operator::Not:
					return L"!";
				case SimpleC::Compiler::Operator::PreInc:
					return L"++";
				case SimpleC::Compiler::Operator::PreDec:
					return L"--";
				case SimpleC::Compiler::Operator::PostInc:
					return L"++";
				case SimpleC::Compiler::Operator::PostDec:
					return L"--";
				case SimpleC::Compiler::Operator::Mul:
					return L"*";
				case SimpleC::Compiler::Operator::Div:
					return L"/";
				case SimpleC::Compiler::Operator::Mod:
					return L"%";
				case SimpleC::Compiler::Operator::Add:
					return L"+";
				case SimpleC::Compiler::Operator::Sub:
					return L"-";
				case SimpleC::Compiler::Operator::Lsh:
					return L"<<";
				case SimpleC::Compiler::Operator::Rsh:
					return L">>";
				case SimpleC::Compiler::Operator::Eql:
					return L"==";
				case SimpleC::Compiler::Operator::Neq:
					return L"!=";
				case SimpleC::Compiler::Operator::Greater:
					return L">";
				case SimpleC::Compiler::Operator::Less:
					return L"<";
				case SimpleC::Compiler::Operator::Geq:
					return L">=";
				case SimpleC::Compiler::Operator::Leq:
					return L"<=";
				case SimpleC::Compiler::Operator::BitAnd:
					return L"&";
				case SimpleC::Compiler::Operator::BitXor:
					return L"^";
				case SimpleC::Compiler::Operator::BitOr:
					return L"|";
				case SimpleC::Compiler::Operator::And:
					return L"&&";
				case SimpleC::Compiler::Operator::Or:
					return L"||";
				case SimpleC::Compiler::Operator::Assign:
					return L"=";
				default:
					return L"ERROR";
				}
			}
			virtual void VisitUnaryExpression(UnaryExpressionSyntaxNode & expr_node)
			{
				auto * expr = &expr_node;
				expr->Expression->Accept(*this);
				
				switch (expr->Operator)
				{
				case Operator::Neg:
					if (expr->Expression->Type == ExpressionType::Int)
						expr->Type = ExpressionType::Int;
					else if (expr->Expression->Type == ExpressionType::Double)
						expr->Type = ExpressionType::Double;
					else
						expr->Type = ExpressionType::Error;
					break;
				case Operator::Not:
				case Operator::PostDec:
				case Operator::PostInc:
				case Operator::PreDec:
				case Operator::PreInc:
					if (expr->Expression->Type == ExpressionType::Int)
						expr->Type = ExpressionType::Int;
					else
						expr->Type = ExpressionType::Error;
					break;
				default:
					expr->Type = ExpressionType::Error;
					break;
				}

				if(expr->Type == ExpressionType::Error && expr->Expression->Type != ExpressionType::Error)
					Error(30020, L"Operator " + OperatorToString(expr->Operator) + L" can not be applied to " + expr->Expression->Type.ToString(), expr);
			}
			virtual void VisitVarExpression(VarExpressionSyntaxNode & expr_node)
			{
				auto * expr = &expr_node;
				auto variable_iter = std::find(function->Variables.begin(), function->Variables.end(), expr->Variable);
				if (variable_iter != function->Variables.end())
				{
					expr->Type = variable_iter->Type;
					expr->Type.IsLeftValue = true;
				}
				else
				{
					int index = -1;
					for(size_t i = 0; i < function->Parameters.size(); i++)
					{
						if(function->Parameters[i]->Name == expr->Variable)
						{
							index = static_cast<int>(i);
							break;
						}
					}
					if (index != -1)
					{
						expr->Type = function->Parameters[index]->Type->ToExpressionType();
						expr->Type.IsLeftValue = true;
					}
					else
					{
						index = -1;
						for(size_t i = 0; i < program->Functions.size(); i++)
						{
							if(program->Functions[i]->Name == expr->Variable)
							{
								index = static_cast<int>(i);
								break;
							}
						}
						if (index != -1)
						{
							expr->Type.BaseType = ExpressionType::_Function;
							expr->Type.Func = program->Functions[index].get();
						}
						else
						{
							expr->Type = ExpressionType::Error;
							Error(30021, L"Undefined identifier \"" + expr->Variable + L"\".", expr);
						}
					}
				}
			}
			// These node kinds carry no extra semantic checks in this visitor.
			virtual void VisitParameter(ParameterSyntaxNode & para){}	
			virtual void VisitType(TypeSyntaxNode & type){}
			virtual void VisitDeclrVariable(VarDeclrStatementSyntaxNode::Variable & variable){}
		};

		std::unique_ptr<SyntaxVisitor> CreateSemanticsVisitor(std::vector<CompileError> & errors)
		{
			return std::make_unique<SemanticsVisitor>(errors);
		}
	}
}
