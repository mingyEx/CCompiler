#ifndef SIMPLE_C_SYNTAX_H
#define SIMPLE_C_SYNTAX_H

#include "Basic.h"

namespace SimpleC
{
	namespace Compiler
	{
		using namespace CoreLib::Basic;
		class SyntaxVisitor;
		class FunctionSyntaxNode;

		class ExpressionType
		{
		public:
			enum _BaseType
			{
				_Int, _Double, _Char, _String, _Function, _Void, _Error
			};
			bool IsLeftValue;
			bool IsReference;	//实现引用
			_BaseType BaseType;
			bool IsArray;
			int ArrayLength;
			FunctionSyntaxNode * Func;
			ExpressionType GetBaseType()
			{
				ExpressionType rs;
				rs.IsLeftValue = IsLeftValue;
				rs.BaseType = BaseType;
				rs.IsArray = false;
				rs.IsReference = false;
				rs.ArrayLength = 0;
				rs.Func = Func;
				return rs;
			}
			ExpressionType()
			{
				BaseType = _Int;
				ArrayLength = 0;
				IsArray = false;
				Func = 0;
				IsLeftValue = false;
				IsReference = false;
			}

			ExpressionType(_BaseType baseType)
			{
				BaseType = baseType;
				ArrayLength = 0;
				IsArray = false;
				Func = 0;
				IsLeftValue = false;
				IsReference = false;
			}

			static ExpressionType Int;
			static ExpressionType Double;
			static ExpressionType Char;
			static ExpressionType String;
			static ExpressionType Void;
			static ExpressionType Error;
			bool operator ==(const ExpressionType & type)
			{
				return (type.BaseType == BaseType &&
					type.IsArray == IsArray &&
						type.ArrayLength == ArrayLength &&
						type.Func == Func);
			}

			bool operator !=(const ExpressionType & type)
			{
				return !(this->operator==(type));
			}

			CoreLib::Basic::String ToString();
		};

		class SyntaxNode
		{
		public:
			int Line, Col;
			String FileName;
			virtual void Accept(SyntaxVisitor * visitor) = 0;
		};
		//默认的是私有继承，https://www.zhihu.com/question/425852397
		class TypeSyntaxNode : public SyntaxNode
		{
		public:
			bool IsArray;
			String TypeName;
			int ArrayLength;
			virtual void Accept(SyntaxVisitor * visitor);
			TypeSyntaxNode()
			{
				ArrayLength = 0;
				IsArray = false;
			}
			ExpressionType ToExpressionType()
			{
				ExpressionType expType;
				if (TypeName == "int")
					expType.BaseType = ExpressionType::_Int;
				else if (TypeName == "double")
					expType.BaseType = ExpressionType::_Double;
				else if (TypeName == "char")
					expType.BaseType = ExpressionType::_Char;
				else if (TypeName == "string")
					expType.BaseType = ExpressionType::_String;
				else if (TypeName == "void")
					expType.BaseType = ExpressionType::_Void;

				expType.ArrayLength = ArrayLength;
				expType.IsArray = IsArray;
				return expType;
			}
		};

		class ParameterSyntaxNode : public SyntaxNode
		{
		public:
			RefPtr<TypeSyntaxNode> Type;
			String Name;
			virtual void Accept(SyntaxVisitor * visitor);
			/*ParameterSyntaxNode(RefPtr<TypeSyntaxNode> type, String name)
				:Type(type), Name(name)
			{
			}*/
			/*ParameterSyntaxNode(int line, int col)
			{
				Line = line;
				Col = col;
			}*/
		};

		enum class ExpressionAccess
		{
			Read, Write
		};

		class ExpressionSyntaxNode : public SyntaxNode
		{
		public:
			ExpressionType Type;
			ExpressionAccess Access;
			ExpressionSyntaxNode()
			{
				Access = ExpressionAccess::Read;
			}
		};
		
		class VarExpressionSyntaxNode : public ExpressionSyntaxNode
		{
		public:
			String Variable;	//var name.
			virtual void Accept(SyntaxVisitor * visitor);
		};

		class ConstantExpressionSyntaxNode : public ExpressionSyntaxNode
		{
		public:
			enum class ConstantType
			{
				Int, Double, Char, String
			};
			ConstantType ConstType;
			union
			{
				int IntValue;
				double DoubleValue;
				wchar_t CharValue;
			};
			String StringValue;
			virtual void Accept(SyntaxVisitor * visitor);
		};

		enum class Operator
		{
			Neg, Not, PreInc, PreDec, PostInc, PostDec,
			Mul, Div, Mod,
			Add, Sub, 
			Lsh, Rsh,
			Eql, Neq, Greater, Less, Geq, Leq,
			BitAnd, BitXor, BitOr,
			And,
			Or,
			Assign
		};
		
		class UnaryExpressionSyntaxNode : public ExpressionSyntaxNode
		{
		public:
			Operator Operator;
			RefPtr<ExpressionSyntaxNode> Expression;
			virtual void Accept(SyntaxVisitor * visitor);
		};
		
		class BinaryExpressionSyntaxNode : public ExpressionSyntaxNode
		{
		public:
			Operator Operator;
			RefPtr<ExpressionSyntaxNode> LeftExpression;
			RefPtr<ExpressionSyntaxNode> RightExpression;
			virtual void Accept(SyntaxVisitor * visitor);
		};

		class IndexExpressionSyntaxNode : public ExpressionSyntaxNode
		{
		public:
			RefPtr<ExpressionSyntaxNode> BaseExpression;
			RefPtr<ExpressionSyntaxNode> IndexExpression;
			
			virtual void Accept(SyntaxVisitor * visitor);
		};

		class InvokeExpressionSyntaxNode : public ExpressionSyntaxNode
		{
		public:
			RefPtr<ExpressionSyntaxNode> FunctionExpr;
			List<RefPtr<ExpressionSyntaxNode>> Arguments;
			virtual void Accept(SyntaxVisitor * visitor);
		};

		class StatementSyntaxNode : public SyntaxNode
		{
		};

		class EmptyStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			virtual void Accept(SyntaxVisitor * visitor);
		};

		class BlockStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			List<RefPtr<StatementSyntaxNode>> Statements;
			virtual void Accept(SyntaxVisitor * visitor);
		};

		class VariableDeclr
		{
		public:
			ExpressionType Type;
			String Name;

			bool operator ==(const VariableDeclr & var)
			{
				return Name == var.Name;
			}
			bool operator ==(const String & name)	// usage?
			{
				return name == Name;
			}
		};
		class FunctionSyntaxNode : public SyntaxNode
		{
		public:
			String Name;
			RefPtr<TypeSyntaxNode> ReturnType;
			List<RefPtr<ParameterSyntaxNode>> Parameters;
			RefPtr<BlockStatementSyntaxNode> Body;
			List<VariableDeclr> Variables;
			virtual void Accept(SyntaxVisitor * visitor);
		};

		class ProgramSyntaxNode : public SyntaxNode
		{
		public:
			ProgramSyntaxNode()//init
			{
				Line = 0;
				Col = 0;
			}
			List<RefPtr<FunctionSyntaxNode>> Functions;
			virtual void Accept(SyntaxVisitor * visitor);
		};

		//"int a=10;"
		class VarDeclrStatementSyntaxNode : public StatementSyntaxNode 
		{
		public:
			struct Variable : public SyntaxNode
			{
				String Name; //var name
				RefPtr<ExpressionSyntaxNode> Expression;	//the part after `=`
				virtual void Accept(SyntaxVisitor * visitor);
			};
			RefPtr<TypeSyntaxNode> Type;
			List<RefPtr<Variable>> Variables;
			virtual void Accept(SyntaxVisitor * visitor);
		};

		class IfStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			RefPtr<ExpressionSyntaxNode> Predicate;
			RefPtr<StatementSyntaxNode> PositiveStatement;
			RefPtr<StatementSyntaxNode> NegativeStatement;
			virtual void Accept(SyntaxVisitor * visitor);
		};

		//看书时间...书上没有，这个估计得看c语言的文法
		class ForStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			RefPtr<ExpressionSyntaxNode> InitialExpression;
			RefPtr<VarDeclrStatementSyntaxNode> VarDeclr;
			RefPtr<ExpressionSyntaxNode> MarginExpression;	// "..;i<n.size();.."
			RefPtr<ExpressionSyntaxNode> SideEffectExpression;	//"++i"
			RefPtr<StatementSyntaxNode> Statement;//"{...}"
			virtual void Accept(SyntaxVisitor * visitor);
		};

		class WhileStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			RefPtr<ExpressionSyntaxNode> Predicate;
			RefPtr<StatementSyntaxNode> Statement;
			virtual void Accept(SyntaxVisitor * visitor);
		};

		class DoWhileStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			RefPtr<StatementSyntaxNode> Statement;
			RefPtr<ExpressionSyntaxNode> Predicate;
			virtual void Accept(SyntaxVisitor * visitor);
		};

		class BreakStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			virtual void Accept(SyntaxVisitor * visitor);
		};

		class ContinueStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			virtual void Accept(SyntaxVisitor * visitor);
		};

		class ReturnStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			RefPtr<ExpressionSyntaxNode> Expression;
			virtual void Accept(SyntaxVisitor * visitor);
		};

		class ExpressionStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			RefPtr<ExpressionSyntaxNode> Expression;
			virtual void Accept(SyntaxVisitor * visitor);
		};

		class SyntaxVisitor
		{
		public:
			virtual void VisitProgram(ProgramSyntaxNode * program)
			{
				program->Functions.ForEach([&](RefPtr<FunctionSyntaxNode> f){f->Accept(this);});	//iterator every function.
			}
			virtual void VisitFunction(FunctionSyntaxNode* function){}	
			virtual void VisitBlockStatement(BlockStatementSyntaxNode* stmt){}
			virtual void VisitBreakStatement(BreakStatementSyntaxNode* stmt){}
			virtual void VisitContinueStatement(ContinueStatementSyntaxNode* stmt){}
			virtual void VisitDoWhileStatement(DoWhileStatementSyntaxNode* stmt){}
			virtual void VisitEmptyStatement(EmptyStatementSyntaxNode* stmt){}
			virtual void VisitForStatement(ForStatementSyntaxNode* stmt){}
			virtual void VisitIfStatement(IfStatementSyntaxNode* stmt){}
			virtual void VisitReturnStatement(ReturnStatementSyntaxNode* stmt){}
			virtual void VisitVarDeclrStatement(VarDeclrStatementSyntaxNode* stmt){}
			virtual void VisitWhileStatement(WhileStatementSyntaxNode* stmt){}
			virtual void VisitExpressionStatement(ExpressionStatementSyntaxNode* stmt){}
			virtual void VisitBinaryExpression(BinaryExpressionSyntaxNode* expr){}
			virtual void VisitConstantExpression(ConstantExpressionSyntaxNode* expr){}
			virtual void VisitIndexExpression(IndexExpressionSyntaxNode* expr){}
			virtual void VisitInvokeExpression(InvokeExpressionSyntaxNode* expr){}
			virtual void VisitUnaryExpression(UnaryExpressionSyntaxNode* expr){}
			virtual void VisitVarExpression(VarExpressionSyntaxNode* expr){}
			virtual void VisitParameter(ParameterSyntaxNode* para){}
			virtual void VisitType(TypeSyntaxNode* type){}
			virtual void VisitDeclrVariable(VarDeclrStatementSyntaxNode::Variable* variable){}
		};
	}
}

#endif