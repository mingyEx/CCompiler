#include "SyntaxVisitors.h"
#include "CodeGenerator.h"
#include "IL/IntermediateCode.h"

namespace SimpleC
{
	namespace Compiler
	{
		void CodeGenerator::VisitProgram(ProgramSyntaxNode * program)
		{
			this->program = program;
			CompiledCode = new Program();
			for (auto & f : program->Functions)
			{
				CompiledCode->Functions.Add(Function());
				curFunc = &CompiledCode->Functions.Last();
				f->Accept(this);
			}
		}
		int GetVarSizeInBytes(ExpressionType & type)
		{
			int arrLen = type.ArrayLength == 0?1:type.ArrayLength;
			switch (type.BaseType)
			{
			case ExpressionType::_Int:
				return 4 * arrLen;
			case ExpressionType::_Double:
				return 8 * arrLen;
			case ExpressionType::_Char:
				return (2 * arrLen + 2)/4 * 4; // 4 bytes alignment
			case ExpressionType::_String:
				throw NotSupportedException(L"string not supported by current back end.");
				return 4;
			default:
				return 0;
				break;
			}
		}
		int GetTypeId(const ExpressionType & type)
		{
			if (type.ArrayLength > 0)
				throw NotSupportedException(L"unsupported type.");
			switch (type.BaseType)
			{
			case ExpressionType::_Int:
				return 1;
			case ExpressionType::_Double:
				return 3;
			case ExpressionType::_Char:
				return 2;
			case ExpressionType::_String:
				throw NotSupportedException(L"string not supported by current back end.");
				return 4;
			default:
				throw NotSupportedException(L"unsupported type.");
				break;
			}
		}
		int GetTypeSizeInBytes(const ExpressionType & type)
		{
			if (type.ArrayLength > 0)
				return 4;
			switch (type.BaseType)
			{
			case ExpressionType::_Int:
				return 4;
			case ExpressionType::_Double:
				return 8;
			case ExpressionType::_Char:
				return 2;
			case ExpressionType::_String:
				throw NotSupportedException(L"string not supported by current back end.");
				return 4;
			default:
				return 0;
				break;
			}
		}

		void CodeGenerator::InitTempVarStack()
		{
			doubleVars.Clear();
			intVars.Clear();
			doubleVarStackPtr = intVarStackPtr = 0;
		}
		Variable * CodeGenerator::AllocVar32()
		{
			Variable * rs = 0;
			if (intVarStackPtr < intVars.Count())
			{
				rs = intVars[intVarStackPtr++];
			}
			else
			{
				rs = curFunc->AddVariable(String(L"^tmp32_") + String(intVarStackPtr++), 0, 4);
				intVars.Add(rs);
			}
			PushExprVar(rs);
			return rs;
		}
		Variable * CodeGenerator::AllocVar64()
		{
			Variable * rs = 0;
			if (doubleVarStackPtr < doubleVars.Count())
			{
				rs = doubleVars[doubleVarStackPtr++];
			}
			else
			{
				rs = curFunc->AddVariable(String(L"^tmp64_") + String(doubleVarStackPtr++), 0, 8);
				doubleVars.Add(rs);
			}
			PushExprVar(rs);
			return rs;
		}
		void CodeGenerator::PushExprVar(Variable * var)
		{
			opStack.Add(var);
		}
		void CodeGenerator::PushExpr(Operand op)
		{
			opStack.Add(op);
		}
		void CodeGenerator::PushExprInt(int val)
		{
			opStack.Add(Operand(val));
		}
		void CodeGenerator::PushExprDouble(double val)
		{
			opStack.Add(Operand(val));
		}
		bool CodeGenerator::IsTempVar(Variable * var)
		{
			return var->Name.Length()>0 && var->Name[0] == L'^';
		}
		Operand CodeGenerator::PopExpr(const ExpressionType & type)
		{
			auto var = opStack.Last();
			opStack.SetSize(opStack.Count()-1);

			if (var.IsVariable() && IsTempVar(var.Var))
			{
				int size = GetTypeSizeInBytes(type);
				if (size<=4)
					if (intVarStackPtr > 0 && intVars[intVarStackPtr-1]->Id == var.Var->Id)
						--intVarStackPtr;
				if (size == 8)
					if (doubleVarStackPtr > 0 && doubleVars[doubleVarStackPtr-1]->Id == var.Var->Id)
						--doubleVarStackPtr;
			}
			return var;
		}

		void CodeGenerator::VisitFunction(FunctionSyntaxNode* function)
		{
			InitTempVarStack();
			loopLabels.Clear();
			labelPosMap.Clear();
			curVars.Clear();
			curFunc->Name = function->Name;
			curFunc->ParameterSize = 0;
			for (auto & param : function->Parameters)
			{
				VarEntry entry;
				entry.Var = curFunc->AddParameter(param->Name, 0, GetVarSizeInBytes(param->Type->ToExpressionType()));
				curFunc->ParameterSize += entry.Var->Size;
				entry.IsParameter = true;
				curVars[param->Name] = entry;
			}
			for (auto & var : function->Variables)
			{
				VarEntry entry;
				
				if (var.Type.IsArray)
				{
					auto ptrVar = entry.Var = curFunc->AddVariable(var.Name, 0, GetVarSizeInBytes(ExpressionType::Int));
					entry.IsParameter = false;
					curVars[var.Name] = entry;
					String arrName = L"^" + var.Name + L"_arr";
					entry.Var = curFunc->AddVariable(arrName, 0, GetVarSizeInBytes(var.Type));
					entry.IsParameter = false;
					curVars[arrName] = entry;	
					Emit(Instruction(ptrVar, Operation::Lea, entry.Var));
				}
				else
				{
					entry.Var = curFunc->AddVariable(var.Name, 0, GetVarSizeInBytes(var.Type));
					entry.IsParameter = false;
					curVars[var.Name] = entry;	
				}
			}
			function->Body->Accept(this);
			Emit(Instruction(Operation::Ret, Operand(curFunc->ParameterSize)));
			// Replace jump labels with positions
			for (auto & instr : curFunc->Instructions)
			{
				if (instr.Func == Operation::Jump)
				{
					instr.Operands[0] = Operand(labelPosMap[instr.Operands[0].IntValue]);
				}
				else if (instr.Func == Operation::Branch)
				{
					instr.Operands[1] = Operand(labelPosMap[instr.Operands[1].IntValue]);
				}
			}
		}

		void CodeGenerator::VisitBlockStatement(BlockStatementSyntaxNode* stmt)
		{
			for (auto & node : stmt->Statements)
			{
				node->Accept(this);
			}
		}
		void CodeGenerator::VisitBreakStatement(BreakStatementSyntaxNode* stmt)
		{
			Emit(Instruction(Operation::Jump, Operand(GetCurrentLoopBreakLabel())));
		}
		void CodeGenerator::VisitContinueStatement(ContinueStatementSyntaxNode* stmt)
		{
			Emit(Instruction(Operation::Jump, Operand(GetCurrentLoopContinueLabel())));
		}
		void CodeGenerator::VisitDoWhileStatement(DoWhileStatementSyntaxNode* stmt)
		{
			PushLoopLabels();
			SetCurrentLoopBeginPosition(GetCurrentInstructionPos());
			stmt->Statement->Accept(this);
			SetCurrentLoopContinuePosition(GetCurrentInstructionPos());
			stmt->Predicate->Accept(this);
			Emit(Instruction(Operation::Branch, PopExpr(stmt->Predicate->Type), GetCurrentLoopBeginLabel(), 0));
			SetCurrentLoopBreakPosition(GetCurrentInstructionPos());
			PopLoopLabels();
		}
		void CodeGenerator::VisitEmptyStatement(EmptyStatementSyntaxNode* stmt){}
		void CodeGenerator::VisitForStatement(ForStatementSyntaxNode* stmt)
		{
			if (stmt->InitialExpression)
				stmt->InitialExpression->Accept(this);
			else if (stmt->VarDeclr)
				stmt->VarDeclr->Accept(this);
			PushLoopLabels();
			SetCurrentLoopBeginPosition(GetCurrentInstructionPos());

			if (stmt->MarginExpression)
				stmt->MarginExpression->Accept(this);
			Emit(Instruction(Operation::Branch, PopExpr(stmt->MarginExpression->Type), GetCurrentLoopBreakLabel(), 1));
			stmt->Statement->Accept(this);
			SetCurrentLoopContinuePosition(GetCurrentInstructionPos());
			if (stmt->SideEffectExpression)
			{
				stmt->SideEffectExpression->Accept(this);
				PopExpr(stmt->SideEffectExpression->Type);
			}
			Emit(Instruction(Operation::Jump, GetCurrentLoopBeginLabel()));
			SetCurrentLoopBreakPosition(GetCurrentInstructionPos());

			PopLoopLabels();
		}
		void CodeGenerator::VisitIfStatement(IfStatementSyntaxNode* stmt)
		{
			stmt->Predicate->Accept(this);
			int endLabel = labelPosMap.Count();
			labelPosMap.Add(-1);
			int elseLabel = labelPosMap.Count();
			labelPosMap.Add(-1);
			Emit(Instruction(Operation::Branch, PopExpr(stmt->Predicate->Type), elseLabel, 1));
			stmt->PositiveStatement->Accept(this);
			if (stmt->NegativeStatement)
			{
				Emit(Instruction(Operation::Jump, endLabel));
				labelPosMap[elseLabel] = GetCurrentInstructionPos();
				stmt->NegativeStatement->Accept(this);
			}
			else
				labelPosMap[elseLabel] = GetCurrentInstructionPos();
			labelPosMap[endLabel] = GetCurrentInstructionPos();
		}
		void CodeGenerator::VisitReturnStatement(ReturnStatementSyntaxNode* stmt)
		{
			stmt->Expression->Accept(this);
			Emit(Instruction(Operation::Ret, Operand(curFunc->ParameterSize), PopExpr(stmt->Expression->Type)));
		}
		void CodeGenerator::VisitVarDeclrStatement(VarDeclrStatementSyntaxNode* stmt)
		{
			for (int i = 0; i < stmt->Variables.Count(); i++)
			{
				auto & variable = stmt->Variables[i];
				if (variable->Expression)
				{
					variable->Expression->Accept(this);
					Emit(Instruction(Operand(curVars[variable->Name].GetValue().Var), 0, PopExpr(variable->Expression->Type)));
				}
			}
		}
		void CodeGenerator::VisitWhileStatement(WhileStatementSyntaxNode* stmt)
		{
			PushLoopLabels();
			SetCurrentLoopBeginPosition(GetCurrentInstructionPos());
			SetCurrentLoopContinuePosition(GetCurrentInstructionPos());
			stmt->Predicate->Accept(this);
			Emit(Instruction(Operation::Branch, PopExpr(stmt->Predicate->Type), GetCurrentLoopBreakLabel(), 1));
			stmt->Statement->Accept(this);
			Emit(Instruction(Operation::Jump, GetCurrentLoopBeginLabel()));
			SetCurrentLoopBreakPosition(GetCurrentInstructionPos());
			PopLoopLabels();
		}
		void CodeGenerator::VisitExpressionStatement(ExpressionStatementSyntaxNode* stmt)
		{
			if (stmt->Expression)
			{
				stmt->Expression->Accept(this);
				if (stmt->Expression->Type != ExpressionType::Void)
					PopExpr(stmt->Expression->Type);
			}
		}

		void CodeGenerator::VisitLogicExpr(BinaryExpressionSyntaxNode * expr)
		{
			auto & leftType = expr->LeftExpression->Type;
			auto & rightType = expr->RightExpression->Type;
			expr->LeftExpression->Accept(this);
			auto leftVar = PopExpr(leftType);
			auto rs = AllocVar32();
			Emit(Instruction(rs, 0, leftVar));
			int labelResult = labelPosMap.Count();
			labelPosMap.Add(-1);
			Emit(Instruction(Operation::Branch, leftVar, labelResult, expr->Operator == Operator::And));
			expr->RightExpression->Accept(this);
			Emit(Instruction(rs, 0, PopExpr(rightType)));
			labelPosMap[labelResult] = curFunc->Instructions.Count();
		}

		void CodeGenerator::VisitBinaryExpression(BinaryExpressionSyntaxNode* expr)
		{
			auto & leftType = expr->LeftExpression->Type;
			auto & rightType = expr->RightExpression->Type;
			auto & type = leftType;
			if (expr->Operator == Operator::And || expr->Operator == Operator::Or)
			{
				VisitLogicExpr(expr);
				return;
			}
			Operand leftVar, rightVar;
			expr->LeftExpression->Accept(this);
			if (expr->Operator != Operator::Assign && leftType == ExpressionType::Int && rightType == ExpressionType::Double)
			{
				Emit(Instruction(AllocVar64(), Operation::I2D, PopExpr(leftType)));
				type = rightType;
			}
			expr->RightExpression->Accept(this);
			if (leftType == ExpressionType::Double && rightType == ExpressionType::Int)
			{
				Emit(Instruction(AllocVar64(), Operation::I2D, PopExpr(rightType)));
			}
			else if (expr->Operator == Operator::Assign && leftType == ExpressionType::Int && rightType == ExpressionType::Double)
			{
				Emit(Instruction(AllocVar32(), Operation::D2I, PopExpr(rightType)));
			}
			rightVar = PopExpr(type);
			leftVar = PopExpr(type);
			switch (expr->Operator)
			{
			case Operator::Add:
				if (type == ExpressionType::Int)
					Emit(Instruction(AllocVar32(), Operation::Add, leftVar, rightVar));
				else if (type == ExpressionType::Double)
					Emit(Instruction(AllocVar64(), Operation::FAdd, leftVar, rightVar));
				else if (type == ExpressionType::String)
					throw NotSupportedException(L"string not supported");
				break;
			case Operator::Sub:
				if (type == ExpressionType::Int)
					Emit(Instruction(AllocVar32(), Operation::Sub, leftVar, rightVar));
				else if (type == ExpressionType::Double)
					Emit(Instruction(AllocVar64(), Operation::FSub, leftVar, rightVar));
				break;
			case Operator::Mul:
				if (type == ExpressionType::Int)
					Emit(Instruction(AllocVar32(), Operation::Mul, leftVar, rightVar));
				else if (type == ExpressionType::Double)
					Emit(Instruction(AllocVar64(), Operation::FMul, leftVar, rightVar));
				break;
			case Operator::Div:
				if (type == ExpressionType::Int)
					Emit(Instruction(AllocVar32(), Operation::Div, leftVar, rightVar));
				else if (type == ExpressionType::Double)
					Emit(Instruction(AllocVar64(), Operation::FDiv, leftVar, rightVar));
				break;
			case Operator::Mod:
				Emit(Instruction(AllocVar32(), Operation::Mod, leftVar, rightVar));
				break;
			case Operator::And:
				Emit(Instruction(AllocVar32(), Operation::And, leftVar, rightVar));
				break;
			case Operator::Or:
				Emit(Instruction(AllocVar32(), Operation::Or, leftVar, rightVar));
				break;
			case Operator::BitAnd:
				Emit(Instruction(AllocVar32(), Operation::BitAnd, leftVar, rightVar));
				break;
			case Operator::BitOr:
				Emit(Instruction(AllocVar32(), Operation::BitOr, leftVar, rightVar));
				break;
			case Operator::BitXor:
				Emit(Instruction(AllocVar32(), Operation::BitXor, leftVar, rightVar));
				break;
			case Operator::Rsh:
				Emit(Instruction(AllocVar32(), Operation::Rsh, leftVar, rightVar));
				break;
			case Operator::Lsh:
				Emit(Instruction(AllocVar32(), Operation::Lsh, leftVar, rightVar));
				break;
			case Operator::Eql:
				Emit(Instruction(AllocVar32(), Operation::Eql, leftVar, rightVar));
				break;
			case Operator::Neq:
				Emit(Instruction(AllocVar32(), Operation::Neq, leftVar, rightVar));
				break;
			case Operator::Geq:
				Emit(Instruction(AllocVar32(), Operation::Geq, leftVar, rightVar));
				break;
			case Operator::Leq:
				Emit(Instruction(AllocVar32(), Operation::Leq, leftVar, rightVar));
				break;
			case Operator::Less:
				Emit(Instruction(AllocVar32(), Operation::Less, leftVar, rightVar));
				break;
			case Operator::Greater:
				Emit(Instruction(AllocVar32(), Operation::Greater, leftVar, rightVar));
				break;
			case Operator::Assign:
				if (!leftType.IsReference)
				{
					Emit(Instruction(leftVar, 0, rightVar));
					PushExpr(leftVar);
				}
				else
				{
					Emit(Instruction(Operand(), Operation::Store, leftVar, rightVar, Operand(GetTypeId(expr->Type))));
					PushExpr(leftVar);
				}
				break;
			default:
				break;
			}
		}
		void CodeGenerator::VisitConstantExpression(ConstantExpressionSyntaxNode* expr)
		{
			switch (expr->ConstType)
			{
			case ConstantExpressionSyntaxNode::ConstantType::Char:
			case ConstantExpressionSyntaxNode::ConstantType::Int:
				PushExprInt(expr->IntValue);
				break;
			case ConstantExpressionSyntaxNode::ConstantType::Double:
				PushExprDouble(expr->DoubleValue);
				break;
			case ConstantExpressionSyntaxNode::ConstantType::String:
				throw NotSupportedException(L"string is not supported");
				break;
			default:
				break;
			}
		}
		void CodeGenerator::VisitIndexExpression(IndexExpressionSyntaxNode* expr)
		{
			expr->BaseExpression->Accept(this);
			expr->IndexExpression->Accept(this);
			auto idx = PopExpr(expr->IndexExpression->Type);
			auto leftOp = PopExpr(expr->BaseExpression->Type);
			Emit(Instruction(AllocVar32(), Operation::SInc, leftOp, idx, Operand(GetTypeSizeInBytes(expr->BaseExpression->Type.GetBaseType()))));
			if (expr->Access == ExpressionAccess::Read)
			{
				auto addr = PopExpr(ExpressionType::Int);
				Emit(Instruction(AllocVar32(), Operation::Load, addr, Operand(GetTypeId(expr->BaseExpression->Type.GetBaseType()))));
			}
		}
		void CodeGenerator::VisitInvokeExpression(InvokeExpressionSyntaxNode* expr)
		{
			List<Operand> arguments;
			arguments.Reserve(expr->Arguments.Count()+1);
			for (int i = expr->Arguments.Count()-1; i>=0; i--)
			{
				expr->Arguments[i]->Accept(this);
			}
			expr->FunctionExpr->Accept(this);
			arguments.Add(PopExpr(expr->FunctionExpr->Type));
			for (int i = 0; i<expr->Arguments.Count(); i++)
			{
				arguments.Add(PopExpr(expr->Arguments[i]->Type));
			}
			if (expr->Type.IsArray)
				throw NotSupportedException(L"back end does not support returning arrays.");
			Variable * rs = 0;
			if (expr->Type == ExpressionType::Int)
				rs = AllocVar32();
			else if (expr->Type == ExpressionType::Double)
				rs = AllocVar64();
			else if (expr->Type != ExpressionType::Void)
				throw NotSupportedException(L"unsupported function return type.");
			if (rs)
				Emit(Instruction(rs, Operation::Call, _Move(arguments)));
			else
				Emit(Instruction(Operand(), Operation::Call, _Move(arguments)));
		}
		void CodeGenerator::VisitUnaryExpression(UnaryExpressionSyntaxNode* expr)
		{
			expr->Expression->Accept(this);
			auto op = PopExpr(expr->Expression->Type);
			switch (expr->Operator)
			{
			case Operator::Neg:
				Emit(Instruction(AllocVar32(), Operation::Neg, op));
				break;
			case Operator::Not:
				Emit(Instruction(AllocVar32(), Operation::Not, op));
				break;
			case Operator::PostInc:
				Emit(Instruction(AllocVar32(), 0, op));
				Emit(Instruction(op, Operation::Add, op, 1));
				break;
			case Operator::PostDec:
				Emit(Instruction(AllocVar32(), 0, op));
				Emit(Instruction(op, Operation::Sub, op, 1));
				break;
			case Operator::PreInc:
				Emit(Instruction(op, Operation::Add, op, 1));
				PushExpr(op);
				break;
			case Operator::PreDec:
				Emit(Instruction(op, Operation::Sub, op, 1));
				PushExpr(op);
				break;
			default:
				throw NotSupportedException(L"Unsupported unary operator");
				break;
			}
		}
		void CodeGenerator::VisitVarExpression(VarExpressionSyntaxNode* expr)
		{
			VarEntry var;

			if (curVars.TryGetValue(expr->Variable, var))
			{
				PushExprVar(var.Var);
			}
			else
			{
				for (int i = 0; i<program->Functions.Count(); i++)
					if (program->Functions[i]->Name == expr->Variable)
					{
						PushExprInt(i);
						return;
					}
				throw InvalidProgramException(L"Unkown identifier");
			}
		}
		void CodeGenerator::VisitParameter(ParameterSyntaxNode* param){}
		void CodeGenerator::VisitType(TypeSyntaxNode* type){}
		void CodeGenerator::VisitDeclrVariable(VarDeclrStatementSyntaxNode::Variable* variable){}

		SyntaxVisitor * CreateCodeGenerator()
		{
			return new CodeGenerator();
		}
	}
}
