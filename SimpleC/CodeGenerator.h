#ifndef SIMPLE_C_CODE_GENERATOR_H
#define SIMPLE_C_CODE_GENERATOR_H

#include "SyntaxVisitors.h"
#include "IL/IntermediateCode.h"

using namespace Compiler::Intermediate;

namespace SimpleC
{
	namespace Compiler
	{
		class VariableOffset
		{
		public:
			String Name;
			int Offset;
			ExpressionType Type;
		};


		class CodeGenerator : public SyntaxVisitor
		{
		private:
			ProgramSyntaxNode * program;
			int paramSize;
			int localVarSize;
			List<Variable*> doubleVars;
			List<Variable*> intVars;
			struct VarEntry
			{
				Variable * Var;
				bool IsParameter;
			};
			Dictionary<String, VarEntry> curVars;
			Function * curFunc;
			int doubleVarStackPtr, intVarStackPtr;
			void InitTempVarStack();
			Variable * AllocVar32();
			Variable * AllocVar64();
			List<Operand> opStack;		//实现算符优先级?
			void PushExprVar(Variable * var);
			void PushExpr(Operand op);
			void PushExprInt(int val);
			void PushExprDouble(double val);
			bool IsTempVar(Variable * var);
			Operand PopExpr(const ExpressionType & type);
		private:
			void Emit(Instruction && instr)
			{
				curFunc->Instructions.AddLast(_Move(instr));	//把指令塞到当前func的class所包含的指令链表里,传入的是Instruction，调用处是godegen。
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
			List<int> labelPosMap;
			List<int> loopLabels;
			int GetCurrentLoopBeginLabel()
			{
				return loopLabels.Last();
			}
			int GetCurrentLoopBreakLabel()
			{
				return loopLabels.Last() + 2;
			}
			int GetCurrentLoopContinueLabel()
			{
				return loopLabels.Last() + 1;
			}
			void SetCurrentLoopBeginPosition(int pos)
			{
				labelPosMap[loopLabels.Last()] = pos;
			}
			void SetCurrentLoopBreakPosition(int pos)
			{
				labelPosMap[loopLabels.Last() + 2] = pos;
			}
			void SetCurrentLoopContinuePosition(int pos)
			{
				labelPosMap[loopLabels.Last() + 1] = pos;
			}
			void PushLoopLabels()
			{
				loopLabels.Add(labelPosMap.Count());
				labelPosMap.Add(-1);
				labelPosMap.Add(-1);
				labelPosMap.Add(-1);
			}
			void PopLoopLabels()
			{
				loopLabels.RemoveAt(loopLabels.Count() - 1);
			}
		public:
			RefPtr<Program> CompiledCode;

			virtual void VisitProgram(ProgramSyntaxNode * program);
			
			virtual void VisitFunction(FunctionSyntaxNode* function);
			virtual void VisitBlockStatement(BlockStatementSyntaxNode* stmt);
			virtual void VisitBreakStatement(BreakStatementSyntaxNode* stmt);
			virtual void VisitContinueStatement(ContinueStatementSyntaxNode* stmt);
			virtual void VisitDoWhileStatement(DoWhileStatementSyntaxNode* stmt);
			virtual void VisitEmptyStatement(EmptyStatementSyntaxNode* stmt);
			virtual void VisitForStatement(ForStatementSyntaxNode* stmt);
			virtual void VisitIfStatement(IfStatementSyntaxNode* stmt);
			virtual void VisitReturnStatement(ReturnStatementSyntaxNode* stmt);
			virtual void VisitVarDeclrStatement(VarDeclrStatementSyntaxNode* stmt);
			virtual void VisitWhileStatement(WhileStatementSyntaxNode* stmt);
			virtual void VisitExpressionStatement(ExpressionStatementSyntaxNode* stmt);
			void VisitLogicExpr(BinaryExpressionSyntaxNode * expr);
			virtual void VisitBinaryExpression(BinaryExpressionSyntaxNode* expr);
			virtual void VisitConstantExpression(ConstantExpressionSyntaxNode* expr);
			virtual void VisitIndexExpression(IndexExpressionSyntaxNode* expr);
			virtual void VisitInvokeExpression(InvokeExpressionSyntaxNode* expr);
			virtual void VisitUnaryExpression(UnaryExpressionSyntaxNode* expr);
			virtual void VisitVarExpression(VarExpressionSyntaxNode* expr);
			virtual void VisitParameter(ParameterSyntaxNode* para);
			virtual void VisitType(TypeSyntaxNode* type);
			virtual void VisitDeclrVariable(VarDeclrStatementSyntaxNode::Variable* variable);
		};
	}
}

#endif