#ifndef SIMPLE_C_SYNTAX_H
#define SIMPLE_C_SYNTAX_H

#include <memory>
#include <string>
#include <vector>

namespace SimpleC
{
	namespace Compiler
	{
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

			std::wstring ToString();
		};

		class SyntaxNode
		{
		public:
			int Line, Col;
			std::wstring FileName;
			virtual ~SyntaxNode() = default;
			virtual void Accept(SyntaxVisitor & visitor) = 0;
		};
		//默认的是私有继承，https://www.zhihu.com/question/425852397
		class TypeSyntaxNode : public SyntaxNode
		{
		public:
			bool IsArray;
			std::wstring TypeName;
			int ArrayLength;
			virtual void Accept(SyntaxVisitor & visitor);
			TypeSyntaxNode()
			{
				ArrayLength = 0;
				IsArray = false;
			}
			ExpressionType ToExpressionType()
			{
				ExpressionType expType;
				if (TypeName == L"int")
					expType.BaseType = ExpressionType::_Int;
				else if (TypeName == L"double")
					expType.BaseType = ExpressionType::_Double;
				else if (TypeName == L"char")
					expType.BaseType = ExpressionType::_Char;
				else if (TypeName == L"string")
					expType.BaseType = ExpressionType::_String;
				else if (TypeName == L"void")
					expType.BaseType = ExpressionType::_Void;

				expType.ArrayLength = ArrayLength;
				expType.IsArray = IsArray;
				return expType;
			}
		};

		class ParameterSyntaxNode : public SyntaxNode
		{
		public:
			std::shared_ptr<TypeSyntaxNode> Type;
			std::wstring Name;
			virtual void Accept(SyntaxVisitor & visitor);
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
			std::wstring Variable;	//var name.
			virtual void Accept(SyntaxVisitor & visitor);
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
			std::wstring StringValue;
			virtual void Accept(SyntaxVisitor & visitor);
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
			std::shared_ptr<ExpressionSyntaxNode> Expression;
			virtual void Accept(SyntaxVisitor & visitor);
		};
		
		class BinaryExpressionSyntaxNode : public ExpressionSyntaxNode
		{
		public:
			Operator Operator;
			std::shared_ptr<ExpressionSyntaxNode> LeftExpression;
			std::shared_ptr<ExpressionSyntaxNode> RightExpression;
			virtual void Accept(SyntaxVisitor & visitor);
		};

		class IndexExpressionSyntaxNode : public ExpressionSyntaxNode
		{
		public:
			std::shared_ptr<ExpressionSyntaxNode> BaseExpression;
			std::shared_ptr<ExpressionSyntaxNode> IndexExpression;
			
			virtual void Accept(SyntaxVisitor & visitor);
		};

		class InvokeExpressionSyntaxNode : public ExpressionSyntaxNode
		{
		public:
			std::shared_ptr<ExpressionSyntaxNode> FunctionExpr;
			std::vector<std::shared_ptr<ExpressionSyntaxNode>> Arguments;
			virtual void Accept(SyntaxVisitor & visitor);
		};

		class StatementSyntaxNode : public SyntaxNode
		{
		};

		class EmptyStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			virtual void Accept(SyntaxVisitor & visitor);
		};

		class BlockStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			std::vector<std::shared_ptr<StatementSyntaxNode>> Statements;
			virtual void Accept(SyntaxVisitor & visitor);
		};

		class VariableDeclr
		{
		public:
			ExpressionType Type;
			std::wstring Name;

			bool operator ==(const VariableDeclr & var)
			{
				return Name == var.Name;
			}
			bool operator ==(const std::wstring & name)	// usage?
			{
				return name == Name;
			}
		};
		class FunctionSyntaxNode : public SyntaxNode
		{
		public:
			std::wstring Name;
			std::shared_ptr<TypeSyntaxNode> ReturnType;
			std::vector<std::shared_ptr<ParameterSyntaxNode>> Parameters;
			std::shared_ptr<BlockStatementSyntaxNode> Body;
			std::vector<VariableDeclr> Variables;
			virtual void Accept(SyntaxVisitor & visitor);
		};

		class ProgramSyntaxNode : public SyntaxNode
		{
		public:
			ProgramSyntaxNode()//init
			{
				Line = 0;
				Col = 0;
			}
			std::vector<std::shared_ptr<FunctionSyntaxNode>> Functions;
			virtual void Accept(SyntaxVisitor & visitor);
		};

		//"int a=10;"
		class VarDeclrStatementSyntaxNode : public StatementSyntaxNode 
		{
		public:
			struct Variable : public SyntaxNode
			{
				std::wstring Name; //var name
				std::shared_ptr<ExpressionSyntaxNode> Expression;	//the part after `=`
				virtual void Accept(SyntaxVisitor & visitor);
			};
			std::shared_ptr<TypeSyntaxNode> Type;
			std::vector<std::shared_ptr<Variable>> Variables;
			virtual void Accept(SyntaxVisitor & visitor);
		};

		class IfStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			std::shared_ptr<ExpressionSyntaxNode> Predicate;
			std::shared_ptr<StatementSyntaxNode> PositiveStatement;
			std::shared_ptr<StatementSyntaxNode> NegativeStatement;
			virtual void Accept(SyntaxVisitor & visitor);
		};

		//看书时间...书上没有，这个估计得看c语言的文法
		class ForStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			std::shared_ptr<ExpressionSyntaxNode> InitialExpression;
			std::shared_ptr<VarDeclrStatementSyntaxNode> VarDeclr;
			std::shared_ptr<ExpressionSyntaxNode> MarginExpression;	// "..;i<n.size();.."
			std::shared_ptr<ExpressionSyntaxNode> SideEffectExpression;	//"++i"
			std::shared_ptr<StatementSyntaxNode> Statement;//"{...}"
			virtual void Accept(SyntaxVisitor & visitor);
		};

		class WhileStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			std::shared_ptr<ExpressionSyntaxNode> Predicate;
			std::shared_ptr<StatementSyntaxNode> Statement;
			virtual void Accept(SyntaxVisitor & visitor);
		};

		class DoWhileStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			std::shared_ptr<StatementSyntaxNode> Statement;
			std::shared_ptr<ExpressionSyntaxNode> Predicate;
			virtual void Accept(SyntaxVisitor & visitor);
		};

		class BreakStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			virtual void Accept(SyntaxVisitor & visitor);
		};

		class ContinueStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			virtual void Accept(SyntaxVisitor & visitor);
		};

		class ReturnStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			std::shared_ptr<ExpressionSyntaxNode> Expression;
			virtual void Accept(SyntaxVisitor & visitor);
		};

		class ExpressionStatementSyntaxNode : public StatementSyntaxNode
		{
		public:
			std::shared_ptr<ExpressionSyntaxNode> Expression;
			virtual void Accept(SyntaxVisitor & visitor);
		};

		class SyntaxVisitor
		{
		public:
			virtual ~SyntaxVisitor() = default;
			virtual void VisitProgram(ProgramSyntaxNode & program)
			{
				for (auto & function : program.Functions)
					function->Accept(*this);
			}
			virtual void VisitFunction(FunctionSyntaxNode& function){}	
			virtual void VisitBlockStatement(BlockStatementSyntaxNode& stmt){}
			virtual void VisitBreakStatement(BreakStatementSyntaxNode& stmt){}
			virtual void VisitContinueStatement(ContinueStatementSyntaxNode& stmt){}
			virtual void VisitDoWhileStatement(DoWhileStatementSyntaxNode& stmt){}
			virtual void VisitEmptyStatement(EmptyStatementSyntaxNode& stmt){}
			virtual void VisitForStatement(ForStatementSyntaxNode& stmt){}
			virtual void VisitIfStatement(IfStatementSyntaxNode& stmt){}
			virtual void VisitReturnStatement(ReturnStatementSyntaxNode& stmt){}
			virtual void VisitVarDeclrStatement(VarDeclrStatementSyntaxNode& stmt){}
			virtual void VisitWhileStatement(WhileStatementSyntaxNode& stmt){}
			virtual void VisitExpressionStatement(ExpressionStatementSyntaxNode& stmt){}
			virtual void VisitBinaryExpression(BinaryExpressionSyntaxNode& expr){}
			virtual void VisitConstantExpression(ConstantExpressionSyntaxNode& expr){}
			virtual void VisitIndexExpression(IndexExpressionSyntaxNode& expr){}
			virtual void VisitInvokeExpression(InvokeExpressionSyntaxNode& expr){}
			virtual void VisitUnaryExpression(UnaryExpressionSyntaxNode& expr){}
			virtual void VisitVarExpression(VarExpressionSyntaxNode& expr){}
			virtual void VisitParameter(ParameterSyntaxNode& para){}
			virtual void VisitType(TypeSyntaxNode& type){}
			virtual void VisitDeclrVariable(VarDeclrStatementSyntaxNode::Variable& variable){}
		};
	}
}

#endif
