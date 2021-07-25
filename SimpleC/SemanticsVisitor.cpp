#include "SyntaxVisitors.h"
#include <set>
#include "IL/CompileError.h"

namespace SimpleC
{
	namespace Compiler
	{
		//头文件有一个SyntaxVisitors.h就行了，甚至可以没有!
		class SemanticsVisitor : public SyntaxVisitor
		{
			ProgramSyntaxNode * program;
			FunctionSyntaxNode * function;	//为什么这三个会被特殊提出来?
			List<SyntaxNode *> loops;
			List<CompileError> & errors;

			void Error(int id, const String & text, SyntaxNode * node)
			{
				errors.Add(CompileError(text, node->FileName, id, node->Line, node->Col));
			}
		public:
			SemanticsVisitor(List<CompileError> & _errors)
				:errors(_errors)
			{
			}
			virtual void VisitProgram(ProgramSyntaxNode * program)
			{
				std::set<String> funcNames;
				this->program = program;
				program->Functions.ForEach([&](RefPtr<FunctionSyntaxNode> & func)
				{
					if (funcNames.find(func->Name) != funcNames.end())
						Error(30001, L"Function \'" + func->Name + L"\' redefinition.", func.Ptr());
					else
						funcNames.insert(func->Name);

					func->Accept(this);
				});
			}
			//复习: override 确保该函数为虚函数并覆盖某个基类中的虚函数,final 则是指定不许被派生类覆盖。
			virtual void VisitFunction(FunctionSyntaxNode *function) override 
			{
				this->function = function;

				auto & returnType = function->ReturnType->ToExpressionType();
				if(returnType.BaseType == ExpressionType::_Void && returnType.IsArray)
					Error(30024, L"Function return type can not be 'void' array.", function->ReturnType.Ptr());

				std::set<String> paraNames;
				function->Parameters.ForEach([&](RefPtr<ParameterSyntaxNode> & para)
				{
					if (paraNames.find(para->Name) != paraNames.end())
						Error(30002, L"Parameter \'" + para->Name + L"\' already defined.", para.Ptr());
					else
						paraNames.insert(para->Name);
					if(para->Type->ToExpressionType().BaseType == ExpressionType::_Void)
						Error(30016, L"'void' can not be parameter type.", para.Ptr());
					//para->Accept(this);
				});
				function->Body->Accept(this);	//块作用域的处理
				
				this->function = NULL;
			}
			//class Lambda01
			//{
			//public:
			//	SemanticsVisitor * _this;
			//	int &cdf;
			//	Lambda01(int &_cdf, SemanticsVisitor *_this)
			//		:cdf(_cdf), _this(_this)
			//	{}
			//	void operator ()(RefPtr<StatementSyntaxNode> & node)
			//	{
			//		int xx = cdf;
			//		node->Accept(_this);
			//	}
			//};	
			virtual void VisitBlockStatement(BlockStatementSyntaxNode *stmt)
			{
				//int cdf = 50;
				//Lambda01 lambda(cdf, this);
				//stmt->Statements.ForEach(lambda);
				stmt->Statements.ForEach([&](RefPtr<StatementSyntaxNode> & node)
				{
					node->Accept(this);
				});
			}
			virtual void VisitBreakStatement(BreakStatementSyntaxNode *stmt)
			{
				if (!loops.Count())
					Error(30003, L"'break' must appear inside loop constructs.", stmt);
			}
			virtual void VisitContinueStatement(ContinueStatementSyntaxNode *stmt)
			{
				if (!loops.Count())
					Error(30004, L"'continue' must appear inside loop constructs.", stmt);
			}
			//基于loops.count来判断，大概是因为下面有修改。

			virtual void VisitDoWhileStatement(DoWhileStatementSyntaxNode *stmt)	
				//DoWhileStatementSyntaxNode 有执行语句和谓词
			{
				loops.Add(stmt);	//loops 类型是list<node>.目的是把所有的循环语句放在一起
				if(stmt->Predicate != NULL)	//只要谓词不为空，就尝试Accept。
					stmt->Predicate->Accept(this);
				if(stmt->Predicate->Type != ExpressionType::Error && stmt->Predicate->Type != ExpressionType::Int)
					Error(30005, L"'while': expression must evaluate to int.", stmt);
				stmt->Statement->Accept(this);	//对语句进行语义分析

				loops.RemoveAt(loops.Count() - 1);
			}
			virtual void VisitEmptyStatement(EmptyStatementSyntaxNode *stmt){}
			virtual void VisitForStatement(ForStatementSyntaxNode *stmt)
			{
				loops.Add(stmt);

				if (stmt->VarDeclr != NULL)		
					stmt->VarDeclr->Accept(this);

				if (stmt->InitialExpression != NULL)
					stmt->InitialExpression->Accept(this);

				if (stmt->MarginExpression != NULL)
					stmt->MarginExpression->Accept(this);

				if (stmt->SideEffectExpression != NULL)
					stmt->SideEffectExpression->Accept(this);

				stmt->Statement->Accept(this);

				loops.RemoveAt(loops.Count() - 1);	//loops是什么？
			}
			virtual void VisitIfStatement(IfStatementSyntaxNode *stmt)
			{
				if (stmt->Predicate != NULL)
					stmt->Predicate->Accept(this);
				if (stmt->Predicate->Type != ExpressionType::Error && stmt->Predicate->Type != ExpressionType::Int)
					Error(30006, L"'if': expression must evaluate to int.", stmt);

				if (stmt->PositiveStatement != NULL)
					stmt->PositiveStatement->Accept(this);
				
				if (stmt->NegativeStatement != NULL)
					stmt->NegativeStatement->Accept(this);
			}
			virtual void VisitReturnStatement(ReturnStatementSyntaxNode *stmt)
			{
				if (stmt->Expression == NULL)
				{
					if(function->ReturnType->ToExpressionType() != ExpressionType::Void)
						Error(30006, L"'return' should have an expression.", stmt);
				}
				else
				{
					stmt->Expression->Accept(this);
					if(stmt->Expression->Type != ExpressionType::Error && stmt->Expression->Type != function->ReturnType->ToExpressionType())
						Error(30007, L"Expression type '" + stmt->Expression->Type.ToString() + L"' does not match function's return type '" + function->ReturnType->ToExpressionType().ToString() + L"'", stmt);
				}
			}
			virtual void VisitVarDeclrStatement(VarDeclrStatementSyntaxNode *stmt)
			{
				stmt->Variables.ForEach([&](RefPtr<VarDeclrStatementSyntaxNode::Variable> para)
				{
					VariableDeclr varDeclr;
					varDeclr.Name = para->Name;
					if(function->Variables.IndexOf(varDeclr) != -1)
						Error(30008, L"Variable " + para->Name + L" already defined.", para.Ptr());
					
					varDeclr.Type = stmt->Type->ToExpressionType();
					if(varDeclr.Type.BaseType == ExpressionType::_Void)
						Error(30009, L"Invalid type 'void'.", stmt);
					if(varDeclr.Type.IsArray && varDeclr.Type.ArrayLength <= 0)
						Error(30025, L"Array size must larger than zero.", stmt);

					function->Variables.Add(varDeclr);
					if(para->Expression != NULL)
						para->Expression->Accept(this);
				});
			}
			virtual void VisitWhileStatement(WhileStatementSyntaxNode *stmt)
			{
				loops.Add(stmt);
				stmt->Predicate->Accept(this);
				if (stmt->Predicate->Type != ExpressionType::Error && stmt->Predicate->Type != ExpressionType::Int)	//类型检查
					Error(30010, L"'while': expression must evaluate to int.", stmt);

				stmt->Statement->Accept(this);
				loops.RemoveAt(loops.Count() - 1);
			}
			virtual void VisitExpressionStatement(ExpressionStatementSyntaxNode *stmt)
			{
				stmt->Expression->Accept(this);//访问者模式经典应用. https://zhuanlan.zhihu.com/p/53810286
			}
			virtual void VisitBinaryExpression(BinaryExpressionSyntaxNode *expr)
			{
				expr->LeftExpression->Accept(this);	//这个this是当前visitor,Accept函数是语法节点的，这样递归调用，会递归到把整棵树遍历到叶子节点。
				//想起来那书上说，类型检查自底向上，这个大概不是类型检查。	
				//Accept就只是用来完成递归调用的一个小洞而已，不要想什么“接收状态”之类的垃圾了。
				expr->RightExpression->Accept(this);
				auto & leftType = expr->LeftExpression->Type;
				auto & rightType = expr->RightExpression->Type;
				//根据参数类型来决定此节点本身的类型。这就是自底向上的类型检查了。想想上面,有递归。
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
						Error(30011, L"Only variables or array elements can be assigned a value.", expr->LeftExpression.Ptr());
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
			virtual void VisitConstantExpression(ConstantExpressionSyntaxNode *expr)
			{
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
			virtual void VisitIndexExpression(IndexExpressionSyntaxNode *expr)
			{
				expr->BaseExpression->Accept(this);
				expr->IndexExpression->Accept(this);
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
			virtual void VisitInvokeExpression(InvokeExpressionSyntaxNode *expr)
			{
				expr->FunctionExpr->Accept(this);
				if (expr->FunctionExpr->Type.BaseType == ExpressionType::_Function &&
					expr->FunctionExpr->Type.ArrayLength == 0)
				{
					auto & func = *(expr->FunctionExpr->Type.Func);
					expr->Type = func.ReturnType->ToExpressionType();
					if(expr->Arguments.Count() != func.Parameters.Count())
						Error(30017, L"Function '" + func.Name + L"' does not take " + String(expr->Arguments.Count()) + L"arguments.", expr);
					else
						for(int i = 0; i < expr->Arguments.Count(); i++)
						{
							expr->Arguments[i]->Accept(this);
							auto paraType = func.Parameters[i]->Type->ToExpressionType();
							if(expr->Arguments[i]->Type != ExpressionType::Error &&
								expr->Arguments[i]->Type != paraType)
							{
								if(!(expr->Arguments[i]->Type == ExpressionType::Int && paraType == ExpressionType::Double))
									Error(30018, L"Argument " + String(i+1) + L" does not evaluate to parameter type '" + paraType.ToString() + L"'", expr);
							}//只做了语法检查
						}
				}
				else
				{
					if (expr->FunctionExpr->Type != ExpressionType::Error)
						Error(30015, L"Expression preceeding parenthesis of apparent function call must have function type.", expr);
					expr->Type = ExpressionType::Error;
				}
			}

			String OperatorToString(Operator op)
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
			virtual void VisitUnaryExpression(UnaryExpressionSyntaxNode *expr)
			{
				expr->Expression->Accept(this);
				
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
			virtual void VisitVarExpression(VarExpressionSyntaxNode *expr)
			{
				int index = function->Variables.IndexOf(expr->Variable);
				if (index != -1)
				{
					expr->Type = function->Variables[index].Type;
					expr->Type.IsLeftValue = true;
				}
				else
				{
					index = -1;
					for(int i = 0; i < function->Parameters.Count(); i++)
					{
						if(function->Parameters[i]->Name == expr->Variable)
						{
							index = i;
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
						for(int i = 0; i < program->Functions.Count(); i++)
						{
							if(program->Functions[i]->Name == expr->Variable)
							{
								index = i;
								break;
							}
						}
						if (index != -1)
						{
							expr->Type.BaseType = ExpressionType::_Function;
							expr->Type.Func = program->Functions[index].Ptr();
						}
						else
						{
							expr->Type = ExpressionType::Error;
							Error(30021, L"Undefined identifier \"" + expr->Variable + L"\".", expr);
						}
					}
				}
			}
			//printer 里有重载.
			virtual void VisitParameter(ParameterSyntaxNode *para){}	
			virtual void VisitType(TypeSyntaxNode *type){}
			virtual void VisitDeclrVariable(VarDeclrStatementSyntaxNode::Variable * variable){}
		};

		SyntaxVisitor * CreateSemanticsVisitor(List<CompileError> & errors)
		{
			return new SemanticsVisitor(errors);
		}
	}
}