#include "SyntaxVisitors.h"
#include "CodeGenerator.h"
#include "IL/IntermediateCode.h"

#include <stdexcept>

namespace SimpleC
{
	namespace Compiler
	{
		void CodeGenerator::VisitProgram(ProgramSyntaxNode & program_node)
		{
			auto * program = &program_node;
			this->program = program;
			CompiledCode = std::make_unique<Program>();
			for (auto & f : program->Functions)
			{
				CompiledCode->Functions.emplace_back();
				curFunc = &CompiledCode->Functions.back();
				f->Accept(*this);
			}
		}
		int GetVarSizeInBytes(const ExpressionType & type)
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
				throw std::runtime_error("string not supported by current back end.");
				return 4;
			default:
				return 0;
				break;
			}
		}
		int GetTypeId(const ExpressionType & type)
		{
			if (type.ArrayLength > 0)
				throw std::runtime_error("unsupported type.");
			switch (type.BaseType)
			{
			case ExpressionType::_Int:
				return 1;
			case ExpressionType::_Double:
				return 3;
			case ExpressionType::_Char:
				return 2;
			case ExpressionType::_String:
				throw std::runtime_error("string not supported by current back end.");
				return 4;
			default:
				throw std::runtime_error("unsupported type.");
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
				throw std::runtime_error("string not supported by current back end.");
				return 4;
			default:
				return 0;
				break;
			}
		}
		DataType GetIntermediateDataType(const ExpressionType & type)
		{
			if (type.BaseType == ExpressionType::_Double)
				return DataType::Double;
			return DataType::Int;
		}

		void CodeGenerator::InitTempVarStack()
		{
			doubleVars.clear();
			intVars.clear();
			doubleVarStackPtr = intVarStackPtr = 0;
		}
		Variable * CodeGenerator::AllocVar32()
		{
			Variable * rs = 0;
			if (intVarStackPtr < static_cast<int>(intVars.size()))
			{
				rs = intVars[intVarStackPtr++];
			}
			else
			{
				const std::wstring name = L"^tmp32_" + std::to_wstring(intVarStackPtr++);
				rs = curFunc->AddVariable(name, 0, 4);
				intVars.push_back(rs);
			}
			rs->Type = DataType::Int;
			PushExprVar(rs);
			return rs;
		}
		Variable * CodeGenerator::AllocVar64()
		{
			Variable * rs = 0;
			if (doubleVarStackPtr < static_cast<int>(doubleVars.size()))
			{
				rs = doubleVars[doubleVarStackPtr++];
			}
			else
			{
				const std::wstring name = L"^tmp64_" + std::to_wstring(doubleVarStackPtr++);
				rs = curFunc->AddVariable(name, 0, 8);
				doubleVars.push_back(rs);
			}
			rs->Type = DataType::Double;
			PushExprVar(rs);
			return rs;
		}
		void CodeGenerator::PushExprVar(Variable * var)
		{
			opStack.push_back(var);
		}
		void CodeGenerator::PushExpr(Operand op)
		{
			opStack.push_back(op);
		}
		void CodeGenerator::PushExprInt(int val)
		{
			opStack.push_back(Operand(val));
		}
		void CodeGenerator::PushExprDouble(double val)
		{
			opStack.push_back(Operand(val));
		}
		bool CodeGenerator::IsTempVar(Variable * var)
		{
			return !var->Name.empty() && var->Name[0] == L'^';
		}
		Operand CodeGenerator::PopExpr(const ExpressionType & type)
		{
			auto var = opStack.back();
			opStack.pop_back();

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

		void CodeGenerator::VisitFunction(FunctionSyntaxNode& function_node)
		{
			auto * function = &function_node;
			InitTempVarStack();
			loopLabels.clear();
			labelPosMap.clear();
			curVars.clear();
			curFunc->SetName(function->Name);
			curFunc->ParameterSize = 0;
			for (auto & param : function->Parameters)
			{
				VarEntry entry;
				entry.Var = curFunc->AddParameter(param->Name, 0, GetVarSizeInBytes(param->Type->ToExpressionType()));
				entry.Var->Type = GetIntermediateDataType(param->Type->ToExpressionType());
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
					ptrVar->Type = DataType::Int;
					entry.IsParameter = false;
					curVars[var.Name] = entry;
					std::wstring arrName = L"^" + var.Name + L"_arr";
					entry.Var = curFunc->AddVariable(arrName, 0, GetVarSizeInBytes(var.Type));
					entry.Var->Type = GetIntermediateDataType(var.Type);
					entry.IsParameter = false;
					curVars[arrName] = entry;	
					Emit(Instruction(ptrVar, Operation::Lea, entry.Var));
				}
				else
				{
					entry.Var = curFunc->AddVariable(var.Name, 0, GetVarSizeInBytes(var.Type));
					entry.Var->Type = GetIntermediateDataType(var.Type);
					entry.IsParameter = false;
					curVars[var.Name] = entry;	
				}
			}
			function->Body->Accept(*this);
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

		void CodeGenerator::VisitBlockStatement(BlockStatementSyntaxNode& stmt_node)
		{
			auto * stmt = &stmt_node;
			for (auto & node : stmt->Statements)
			{
				node->Accept(*this);
			}
		}
		void CodeGenerator::VisitBreakStatement(BreakStatementSyntaxNode& stmt)
		{
			Emit(Instruction(Operation::Jump, Operand(GetCurrentLoopBreakLabel())));
		}
		void CodeGenerator::VisitContinueStatement(ContinueStatementSyntaxNode& stmt)
		{
			Emit(Instruction(Operation::Jump, Operand(GetCurrentLoopContinueLabel())));
		}
		void CodeGenerator::VisitDoWhileStatement(DoWhileStatementSyntaxNode& stmt_node)
		{
			auto * stmt = &stmt_node;
			PushLoopLabels();
			SetCurrentLoopBeginPosition(GetCurrentInstructionPos());
			stmt->Statement->Accept(*this);
			SetCurrentLoopContinuePosition(GetCurrentInstructionPos());
			stmt->Predicate->Accept(*this);
			Emit(Instruction(Operation::Branch, PopExpr(stmt->Predicate->Type), GetCurrentLoopBeginLabel(), 0));
			SetCurrentLoopBreakPosition(GetCurrentInstructionPos());
			PopLoopLabels();
		}
		void CodeGenerator::VisitEmptyStatement(EmptyStatementSyntaxNode& stmt){}
		void CodeGenerator::VisitForStatement(ForStatementSyntaxNode& stmt_node)
		{
			auto * stmt = &stmt_node;
			if (stmt->InitialExpression)
				stmt->InitialExpression->Accept(*this);
			else if (stmt->VarDeclr)
				stmt->VarDeclr->Accept(*this);
			PushLoopLabels();
			SetCurrentLoopBeginPosition(GetCurrentInstructionPos());

			if (stmt->MarginExpression)
				stmt->MarginExpression->Accept(*this);
			Emit(Instruction(Operation::Branch, PopExpr(stmt->MarginExpression->Type), GetCurrentLoopBreakLabel(), 1));
			stmt->Statement->Accept(*this);
			SetCurrentLoopContinuePosition(GetCurrentInstructionPos());
			if (stmt->SideEffectExpression)
			{
				stmt->SideEffectExpression->Accept(*this);
				PopExpr(stmt->SideEffectExpression->Type);
			}
			Emit(Instruction(Operation::Jump, GetCurrentLoopBeginLabel()));
			SetCurrentLoopBreakPosition(GetCurrentInstructionPos());

			PopLoopLabels();
		}
		void CodeGenerator::VisitIfStatement(IfStatementSyntaxNode& stmt_node)
		{
			auto * stmt = &stmt_node;
			stmt->Predicate->Accept(*this);
			int endLabel = static_cast<int>(labelPosMap.size());
			labelPosMap.push_back(-1);
			int elseLabel = static_cast<int>(labelPosMap.size());
			labelPosMap.push_back(-1);
			Emit(Instruction(Operation::Branch, PopExpr(stmt->Predicate->Type), elseLabel, 1));
			stmt->PositiveStatement->Accept(*this);
			if (stmt->NegativeStatement)
			{
				Emit(Instruction(Operation::Jump, endLabel));
				labelPosMap[elseLabel] = GetCurrentInstructionPos();
				stmt->NegativeStatement->Accept(*this);
			}
			else
				labelPosMap[elseLabel] = GetCurrentInstructionPos();
			labelPosMap[endLabel] = GetCurrentInstructionPos();
		}
		void CodeGenerator::VisitReturnStatement(ReturnStatementSyntaxNode& stmt_node)
		{
			auto * stmt = &stmt_node;
			stmt->Expression->Accept(*this);
			Emit(Instruction(Operation::Ret, Operand(curFunc->ParameterSize), PopExpr(stmt->Expression->Type)));
		}
		void CodeGenerator::VisitVarDeclrStatement(VarDeclrStatementSyntaxNode& stmt_node)
		{
			auto * stmt = &stmt_node;
			for (size_t i = 0; i < stmt->Variables.size(); i++)
			{
				auto & variable = stmt->Variables[i];
				if (variable->Expression)
				{
					variable->Expression->Accept(*this);
					auto value = PopExpr(variable->Expression->Type);
					auto targetType = stmt->Type->ToExpressionType();
					if (targetType == ExpressionType::Double && variable->Expression->Type == ExpressionType::Int)
					{
						Emit(Instruction(AllocVar64(), Operation::I2D, value));
						value = PopExpr(targetType);
					}
					else if (targetType == ExpressionType::Int && variable->Expression->Type == ExpressionType::Double)
					{
						Emit(Instruction(AllocVar32(), Operation::D2I, value));
						value = PopExpr(targetType);
					}
					Emit(Instruction(Operand(curVars.at(variable->Name).Var), 0, value));
				}
			}
		}
		void CodeGenerator::VisitWhileStatement(WhileStatementSyntaxNode& stmt_node)
		{
			auto * stmt = &stmt_node;
			PushLoopLabels();
			SetCurrentLoopBeginPosition(GetCurrentInstructionPos());
			SetCurrentLoopContinuePosition(GetCurrentInstructionPos());
			stmt->Predicate->Accept(*this);
			Emit(Instruction(Operation::Branch, PopExpr(stmt->Predicate->Type), GetCurrentLoopBreakLabel(), 1));
			stmt->Statement->Accept(*this);
			Emit(Instruction(Operation::Jump, GetCurrentLoopBeginLabel()));
			SetCurrentLoopBreakPosition(GetCurrentInstructionPos());
			PopLoopLabels();
		}
		void CodeGenerator::VisitExpressionStatement(ExpressionStatementSyntaxNode& stmt_node)
		{
			auto * stmt = &stmt_node;
			if (stmt->Expression)
			{
				stmt->Expression->Accept(*this);
				if (stmt->Expression->Type != ExpressionType::Void)
					PopExpr(stmt->Expression->Type);
			}
		}

		void CodeGenerator::VisitLogicExpr(BinaryExpressionSyntaxNode & expr_node)
		{
			auto * expr = &expr_node;
			auto & leftType = expr->LeftExpression->Type;
			auto & rightType = expr->RightExpression->Type;
			expr->LeftExpression->Accept(*this);
			auto leftVar = PopExpr(leftType);
			auto rs = AllocVar32();
			Emit(Instruction(rs, 0, leftVar));
			int labelResult = static_cast<int>(labelPosMap.size());
			labelPosMap.push_back(-1);
			Emit(Instruction(Operation::Branch, leftVar, labelResult, expr->Operator == Operator::And));
			expr->RightExpression->Accept(*this);
			Emit(Instruction(rs, 0, PopExpr(rightType)));
			labelPosMap[labelResult] = curFunc->Instructions.Count();
		}

		void CodeGenerator::VisitBinaryExpression(BinaryExpressionSyntaxNode& expr_node)
		{
			auto * expr = &expr_node;
			auto & leftType = expr->LeftExpression->Type;
			auto & rightType = expr->RightExpression->Type;
			auto & type = leftType;
			if (expr->Operator == Operator::And || expr->Operator == Operator::Or)
			{
				VisitLogicExpr(*expr);
				return;
			}
			Operand leftVar, rightVar;
			expr->LeftExpression->Accept(*this);
			if (expr->Operator != Operator::Assign && leftType == ExpressionType::Int && rightType == ExpressionType::Double)
			{
				Emit(Instruction(AllocVar64(), Operation::I2D, PopExpr(leftType)));
				type = rightType;
			}
			expr->RightExpression->Accept(*this);
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
					throw std::runtime_error("string not supported");
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
		void CodeGenerator::VisitConstantExpression(ConstantExpressionSyntaxNode& expr_node)
		{
			auto * expr = &expr_node;
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
				throw std::runtime_error("string is not supported");
				break;
			default:
				break;
			}
		}
		void CodeGenerator::VisitIndexExpression(IndexExpressionSyntaxNode& expr_node)
		{
			auto * expr = &expr_node;
			expr->BaseExpression->Accept(*this);
			expr->IndexExpression->Accept(*this);
			auto idx = PopExpr(expr->IndexExpression->Type);
			auto leftOp = PopExpr(expr->BaseExpression->Type);
			Emit(Instruction(AllocVar32(), Operation::SInc, leftOp, idx, Operand(GetTypeSizeInBytes(expr->BaseExpression->Type.GetBaseType()))));
			if (expr->Access == ExpressionAccess::Read)
			{
				auto addr = PopExpr(ExpressionType::Int);
				Emit(Instruction(AllocVar32(), Operation::Load, addr, Operand(GetTypeId(expr->BaseExpression->Type.GetBaseType()))));
			}
		}
		void CodeGenerator::VisitInvokeExpression(InvokeExpressionSyntaxNode& expr_node)
		{
			auto * expr = &expr_node;
			std::vector<Operand> arguments;
			arguments.reserve(expr->Arguments.size() + 1);
			for (size_t i = expr->Arguments.size(); i > 0; i--)
			{
				expr->Arguments[i - 1]->Accept(*this);
			}
			expr->FunctionExpr->Accept(*this);
			arguments.push_back(PopExpr(expr->FunctionExpr->Type));
			for (size_t i = 0; i < expr->Arguments.size(); i++)
			{
				arguments.push_back(PopExpr(expr->Arguments[i]->Type));
			}
			if (expr->Type.IsArray)
				throw std::runtime_error("back end does not support returning arrays.");
			Variable * rs = 0;
			if (expr->Type == ExpressionType::Int)
				rs = AllocVar32();
			else if (expr->Type == ExpressionType::Double)
				rs = AllocVar64();
			else if (expr->Type != ExpressionType::Void)
				throw std::runtime_error("unsupported function return type.");
			if (rs)
				Emit(Instruction(rs, Operation::Call, std::move(arguments)));
			else
				Emit(Instruction(Operand(), Operation::Call, std::move(arguments)));
		}
		void CodeGenerator::VisitUnaryExpression(UnaryExpressionSyntaxNode& expr_node)
		{
			auto * expr = &expr_node;
			expr->Expression->Accept(*this);
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
				throw std::runtime_error("Unsupported unary operator");
				break;
			}
		}
		void CodeGenerator::VisitVarExpression(VarExpressionSyntaxNode& expr_node)
		{
			auto * expr = &expr_node;
			if (auto iter = curVars.find(expr->Variable); iter != curVars.end())
			{
				PushExprVar(iter->second.Var);
			}
			else
			{
				for (size_t i = 0; i < program->Functions.size(); i++)
					if (program->Functions[i]->Name == expr->Variable)
					{
						PushExprInt(static_cast<int>(i));
						return;
					}
				throw InvalidProgramException(L"Unkown identifier");
			}
		}
		void CodeGenerator::VisitParameter(ParameterSyntaxNode& param){}
		void CodeGenerator::VisitType(TypeSyntaxNode& type){}
		void CodeGenerator::VisitDeclrVariable(VarDeclrStatementSyntaxNode::Variable& variable){}

		std::unique_ptr<SyntaxVisitor> CreateCodeGenerator()
		{
			return std::make_unique<CodeGenerator>();
		}
	}
}
