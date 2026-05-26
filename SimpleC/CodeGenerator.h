#ifndef SIMPLE_C_CODE_GENERATOR_H
#define SIMPLE_C_CODE_GENERATOR_H

#include "SyntaxVisitors.h"
#include "IL/IntermediateCode.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using namespace Compiler::Intermediate;

namespace SimpleC
{
	namespace Compiler
	{
		class VariableOffset
		{
		public:
			std::wstring Name;
			int Offset;
			ExpressionType Type;
		};


		class CodeGenerator : public SyntaxVisitor
		{
		private:
			ProgramSyntaxNode * program;
			int paramSize;
			int localVarSize;
			std::vector<Variable*> doubleVars;
			std::vector<Variable*> intVars;
			struct VarEntry
			{
				Variable * Var;
				bool IsParameter;
			};
			std::unordered_map<std::wstring, VarEntry> curVars;
			Function * curFunc;
			int doubleVarStackPtr, intVarStackPtr;
			void InitTempVarStack();
			Variable * AllocVar32();
			Variable * AllocVar64();
			std::vector<Operand> opStack;		// temporary operand stack during expression lowering
			void PushExprVar(Variable * var);
			void PushExpr(Operand op);
			void PushExprInt(int val);
			void PushExprDouble(double val);
			bool IsTempVar(Variable * var);
			Operand PopExpr(const ExpressionType & type);
		private:
			void Emit(Instruction && instr)
			{
				curFunc->Instructions.AddLast(std::move(instr));	//把指令塞到当前func的class所包含的指令链表里,传入的是Instruction，调用处是godegen。
			}
			void Emit(const Instruction & instr)
			{
				curFunc->Instructions.AddLast(instr);
			}
			int GetCurrentInstructionPos()
			{
				return curFunc->Instructions.Count();
			}
		private:
			std::vector<int> labelPosMap;
			std::vector<int> loopLabels;
			int GetCurrentLoopBeginLabel()
			{
				return loopLabels.back();
			}
			int GetCurrentLoopBreakLabel()
			{
				return loopLabels.back() + 2;
			}
			int GetCurrentLoopContinueLabel()
			{
				return loopLabels.back() + 1;
			}
			void SetCurrentLoopBeginPosition(int pos)
			{
				labelPosMap[loopLabels.back()] = pos;
			}
			void SetCurrentLoopBreakPosition(int pos)
			{
				labelPosMap[loopLabels.back() + 2] = pos;
			}
			void SetCurrentLoopContinuePosition(int pos)
			{
				labelPosMap[loopLabels.back() + 1] = pos;
			}
			void PushLoopLabels()
			{
				loopLabels.push_back(static_cast<int>(labelPosMap.size()));
				labelPosMap.push_back(-1);
				labelPosMap.push_back(-1);
				labelPosMap.push_back(-1);
			}
			void PopLoopLabels()
			{
				loopLabels.pop_back();
			}
		public:
			std::unique_ptr<Program> CompiledCode;

			virtual void VisitProgram(ProgramSyntaxNode & program);

			virtual void VisitFunction(FunctionSyntaxNode& function);
			virtual void VisitBlockStatement(BlockStatementSyntaxNode& stmt);
			virtual void VisitBreakStatement(BreakStatementSyntaxNode& stmt);
			virtual void VisitContinueStatement(ContinueStatementSyntaxNode& stmt);
			virtual void VisitDoWhileStatement(DoWhileStatementSyntaxNode& stmt);
			virtual void VisitEmptyStatement(EmptyStatementSyntaxNode& stmt);
			virtual void VisitForStatement(ForStatementSyntaxNode& stmt);
			virtual void VisitIfStatement(IfStatementSyntaxNode& stmt);
			virtual void VisitReturnStatement(ReturnStatementSyntaxNode& stmt);
			virtual void VisitVarDeclrStatement(VarDeclrStatementSyntaxNode& stmt);
			virtual void VisitWhileStatement(WhileStatementSyntaxNode& stmt);
			virtual void VisitExpressionStatement(ExpressionStatementSyntaxNode& stmt);
			void VisitLogicExpr(BinaryExpressionSyntaxNode & expr);
			virtual void VisitBinaryExpression(BinaryExpressionSyntaxNode& expr);
			virtual void VisitConstantExpression(ConstantExpressionSyntaxNode& expr);
			virtual void VisitIndexExpression(IndexExpressionSyntaxNode& expr);
			virtual void VisitInvokeExpression(InvokeExpressionSyntaxNode& expr);
			virtual void VisitUnaryExpression(UnaryExpressionSyntaxNode& expr);
			virtual void VisitVarExpression(VarExpressionSyntaxNode& expr);
			virtual void VisitParameter(ParameterSyntaxNode& para);
			virtual void VisitType(TypeSyntaxNode& type);
			virtual void VisitDeclrVariable(VarDeclrStatementSyntaxNode::Variable& variable);
		};
	}
}

#endif
