#include "IntermediateCode.h"

#include <sstream>

namespace Compiler
{
	namespace Intermediate
	{
		namespace
		{
			template <typename TValue>
			std::wstring ToWideString(TValue value)
			{
				std::wostringstream writer;
				writer << value;
				return writer.str();
			}
		}

		class AddSubOperation : public Operation
		{
		public:
			AddSubOperation(const wchar_t * name)
				:Operation(name)
			{}
			virtual bool TryEvaluate(Operand & opReplace, std::vector<Operand> & ops) override
			{
				if (ops[0].IsIntegral() && ops[1].IsIntegral())
				{
					if (this == Operation::Add)
						opReplace = Operand(ops[0].IntValue + ops[1].IntValue);
					else
						opReplace = Operand(ops[0].IntValue - ops[1].IntValue);
					return true;
				}
				else if (this == Operation::Add && ops[0].IsIntegral() && ops[0].IntValue == 0)
				{
					opReplace = ops[1];
					return true;
				}
				else if (ops[1].IsIntegral() && ops[1].IntValue == 0)
				{
					opReplace = ops[0];
					return true;
				}
				else if (this == Operation::Sub && ops[0].IsVariable() && ops[1].IsVariable() && ops[0].Var == ops[1].Var)
				{
					opReplace = Operand(0);
					return true;
				}
				return false;
			}
		};
		class ModOperation : public Operation
		{
		public:
			ModOperation(const wchar_t * name)
				:Operation(name)
			{}
			virtual bool TryEvaluate(Operand & opReplace, std::vector<Operand> & ops) override
			{
				if (ops[0].IsIntegral() && ops[1].IsIntegral())
				{
					if (ops[1].IntValue == 0)
						throw CompileErrorException(L"Division by zero.");
					opReplace = Operand(ops[0].IntValue % ops[1].IntValue);
					return true;
				}
				else if (ops[0].IsIntegral() && ops[0].IntValue == 0)
				{
					opReplace = Operand(0);
					return true;
				}
				else if (ops[1].IsIntegral() && ops[1].IntValue == 1)
				{
					opReplace = Operand(0);
					return true;
				}
				else if (ops[0].IsVariable() && ops[1].IsVariable() && ops[0].Var == ops[1].Var)
				{
					opReplace = Operand(0);
					return true;
				}
				return false;
			}
		};
		class MulDivOperation : public Operation
		{
		public:
			MulDivOperation(const wchar_t * name)
				:Operation(name)
			{}
			virtual bool TryEvaluate(Operand & opReplace, std::vector<Operand> & ops) override
			{
				if (ops[0].IsIntegral() && ops[1].IsIntegral())
				{
					if (ops[1].IntValue == 0)
						throw CompileErrorException(L"Division by zero.");
					if (this == Operation::Mul)
						opReplace = Operand(ops[0].IntValue * ops[1].IntValue);
					else
						opReplace = Operand(ops[0].IntValue / ops[1].IntValue);
					return true;
				}
				else if (ops[0].IsIntegral() && ops[0].IntValue == 0)
				{
					opReplace = Operand(0);
					return true;
				}
				else if (ops[1].IsIntegral() && ops[1].IntValue == 0)
				{
					if (this == Operation::Div)
						throw CompileErrorException(L"Division by zero.");
					opReplace = Operand(0);
					return true;
				}
				else if (ops[0].IsIntegral() && ops[0].IntValue == 1)
				{
					if (this == Operation::Mul)
					{
						opReplace = ops[1];
						return true;
					}
				}
				else if (ops[1].IsIntegral() && ops[1].IntValue == 1)
				{
					opReplace = ops[0];
					return true;
				}
				else if (this == Operation::Div && ops[0].IsVariable() && ops[1].IsVariable() && ops[0].Var == ops[1].Var)
				{
					opReplace = Operand(1);
					return true;
				}
				return false;
			}
		};
		class FAddSubOperation : public Operation
		{
		public:
			FAddSubOperation(const wchar_t * name)
				:Operation(name)
			{}
			virtual bool TryEvaluate(Operand & opReplace, std::vector<Operand> & ops) override
			{
				bool isOpConst[2];
				double opValue[2] = {0.0, 0.0};
				for (int i = 0; i<2; i++)
				{
					isOpConst[i] = ops[i].Type == OperandType::ConstFloat ||
								   ops[i].Type == OperandType::ConstDouble;
					if (ops[i].Type == OperandType::ConstFloat)
						opValue[i] = ops[i].FloatValue;
					else
						opValue[i] = ops[i].DoubleValue;
				}
				bool isOp1Const = ops[1].Type == OperandType::ConstFloat ||
					ops[1].Type == OperandType::ConstDouble;
				
				if (isOpConst[0] && isOpConst[1])
				{
					double rs;
					if (this == Operation::FAdd)
						rs = opValue[0] + opValue[1];
					else
						rs = opValue[0] - opValue[1];
					if (ops[0].Type == OperandType::ConstDouble || ops[1].Type == OperandType::ConstDouble)
						opReplace = Operand(rs);
					else
						opReplace = Operand((float)(rs));
					return true;
				}
				else if (this == Operation::FAdd && isOpConst[0] && opValue[0] == 0.0)
				{
					opReplace = ops[1];
					return true;
				}
				else if (isOpConst[1] && opValue[1] == 0.0)
				{
					opReplace = ops[0];
					return true;
				}
				else if (this == Operation::FSub && ops[0].IsVariable() && ops[1].IsVariable() && ops[0].Var == ops[1].Var)
				{
					if (ops[0].GetDataType() == DataType::Double || ops[1].GetDataType() == DataType::Double)
						opReplace = Operand(0.0);
					else
						opReplace = Operand(0.0f);
					return true;
				}
				return false;
			}
		};
		class FMulDivOperation : public Operation
		{
		public:
			FMulDivOperation(const wchar_t * name)
				:Operation(name)
			{}
			virtual bool TryEvaluate(Operand & opReplace, std::vector<Operand> & ops) override
			{
				bool isOpConst[2];
				double opValue[2] = {0.0, 0.0};
				for (int i = 0; i<2; i++)
				{
					isOpConst[i] = ops[i].Type == OperandType::ConstFloat ||
								   ops[i].Type == OperandType::ConstDouble;
					if (ops[i].Type == OperandType::ConstFloat)
						opValue[i] = ops[i].FloatValue;
					else
						opValue[i] = ops[i].DoubleValue;
				}
				bool isOp1Const = ops[1].Type == OperandType::ConstFloat ||
					ops[1].Type == OperandType::ConstDouble;
				
				if (isOpConst[0] && isOpConst[1])
				{
					double rs;
					if (this == Operation::FDiv && opValue[1] == 0.0)
						throw CompileErrorException(L"Division by zero.");
					if (this == Operation::FMul)
						rs = opValue[0] * opValue[1];
					else
						rs = opValue[0] / opValue[1];
					if (ops[0].Type == OperandType::ConstDouble || ops[1].Type == OperandType::ConstDouble)
						opReplace = Operand(rs);
					else
						opReplace = Operand((float)(rs));
					return true;
				}
				else if (isOpConst[1] && opValue[1] == 1.0)
				{
					opReplace = ops[0];
					return true;
				}
				else if (this == Operation::FMul && isOpConst[0] && opValue[0] == 1.0)
				{
					opReplace = ops[1];
					return true;
				}
				else if (this == Operation::FMul && (isOpConst[0] && opValue[0] == 0.0 || isOpConst[1] && opValue[1] == 0.0))
				{
					if (ops[0].GetDataType() == DataType::Double || ops[1].GetDataType() == DataType::Double)
						opReplace = Operand(0.0);
					else
						opReplace = Operand((float)(0.0f));
					return true;
				}
				else if (this == Operation::FDiv && ops[0].IsVariable() && ops[1].IsVariable() && ops[0].Var == ops[1].Var)
				{
					if (ops[0].GetDataType() == DataType::Double || ops[1].GetDataType() == DataType::Double)
						opReplace = Operand(1.0);
					else
						opReplace = Operand(1.0f);
					return true;
				}
				else if (this == Operation::FDiv && isOpConst[0] && opValue[0] == 0.0)
				{
					if (ops[0].Type == OperandType::ConstDouble || ops[1].Type == OperandType::ConstDouble)
						opReplace = Operand(0.0);
					else
						opReplace = Operand(0.0f);
				}
				else if (this == Operation::FDiv && isOpConst[1] && opValue[1] == 0.0)
				{
					throw CompileErrorException(L"Division by zero.");
				}
				return false;
			}
		};
		class ShiftOperation : public Operation
		{
		public:
			ShiftOperation(const wchar_t * name)
				:Operation(name)
			{}
			virtual bool TryEvaluate(Operand & opReplace, std::vector<Operand> & ops) override
			{
				if (ops[0].IsIntegral() && ops[1].IsIntegral())
				{
					if (this == Operation::Lsh)
						opReplace = ops[0].IntValue << ops[1].IntValue;
					else
						opReplace = ops[0].IntValue >> ops[1].IntValue;
					return true;
				}
				else if (ops[1].IsIntegral() && ops[1].IntValue == 0)
				{
					opReplace = ops[0];
					return true;
				}
				return false;
			}
		};
		class CompareOperation : public Operation
		{
		public:
			CompareOperation(const wchar_t * name)
				:Operation(name)
			{}
			virtual bool TryEvaluate(Operand & opReplace, std::vector<Operand> & ops) override
			{
				if (this == Operation::Compare)
					return false;
				if (ops[0].IsIntegral() && ops[1].IsIntegral())
				{
					if (this == Operation::Eql)
						opReplace = Operand(ops[0].IntValue == ops[1].IntValue);
					else if (this == Operation::Neq)
						opReplace = Operand(ops[0].IntValue != ops[1].IntValue);
					else if (this == Operation::Less)
						opReplace = Operand(ops[0].IntValue < ops[1].IntValue);
					else if (this == Operation::Greater)
						opReplace = Operand(ops[0].IntValue > ops[1].IntValue);
					else if (this == Operation::Leq)
						opReplace = Operand(ops[0].IntValue <= ops[1].IntValue);
					else if (this == Operation::Geq)
						opReplace = Operand(ops[0].IntValue >= ops[1].IntValue);
					return true;
				}
				else if (ops[0].IsVariable() && ops[1].IsVariable() &&
					ops[0].Var == ops[1].Var && ops[0].Var->Type == DataType::Int)
				{
					if (this == Operation::Eql)
						opReplace = Operand(1);
					else if (this == Operation::Leq || this == Operation::Geq)
						opReplace = Operand(1);
					else
						opReplace = Operand(0);
					return true;
				}
				return false;
			}
		};
		class NegNotOperation : public Operation
		{
		public:
			NegNotOperation(const wchar_t * name)
				:Operation(name)
			{}
			virtual bool TryEvaluate(Operand & opReplace, std::vector<Operand> & ops) override
			{
				if (ops[0].IsIntegral())
				{
					if (this == Operation::Neg)
						opReplace = Operand(-ops[0].IntValue);
					else if (this == Operation::Not)
						opReplace = Operand(!ops[0].IntValue);
					else if (this == Operation::BitNot)
						opReplace = Operand(~ops[0].IntValue);
					return true;
				}
				return false;
			}
		};
		class AndOperation : public Operation
		{
		public:
			AndOperation(const wchar_t * name)
				:Operation(name)
			{}
			virtual bool TryEvaluate(Operand & opReplace, std::vector<Operand> & ops) override
			{
				if (ops[0].IsIntegral() && ops[1].IsIntegral())
				{
					opReplace = Operand(ops[0].IntValue && ops[1].IntValue);
					return true;
				}
				else if (ops[0].IsIntegral() && ops[0].IntValue == 0)
				{
					opReplace = Operand(0);
					return true;
				}
				else if (ops[1].IsIntegral() && ops[1].IntValue == 0)
				{
					opReplace = Operand(0);
					return true;
				}
				return false;
			}
		};
		class OrOperation : public Operation
		{
		public:
			OrOperation(const wchar_t * name)
				:Operation(name)
			{}
			virtual bool TryEvaluate(Operand & opReplace, std::vector<Operand> & ops) override
			{
				if (ops[0].IsIntegral() && ops[1].IsIntegral())
				{
					opReplace = Operand(ops[0].IntValue || ops[1].IntValue);
					return true;
				}
				else if (ops[0].IsIntegral() && ops[0].IntValue == 1)
				{
					opReplace = Operand(1);
					return true;
				}
				else if (ops[1].IsIntegral() && ops[1].IntValue == 1)
				{
					opReplace = Operand(1);
					return true;
				}
				return false;
			}
		};
		class BitOperation : public Operation
		{
		public:
			BitOperation(const wchar_t * name)
				:Operation(name)
			{}
			virtual bool TryEvaluate(Operand & opReplace, std::vector<Operand> & ops) override
			{
				if (ops[0].IsIntegral() && ops[1].IsIntegral())
				{
					if (this == Operation::BitAnd)
						opReplace = Operand(ops[0].IntValue & ops[1].IntValue);
					else if (this == Operation::BitOr)
						opReplace = Operand(ops[0].IntValue | ops[1].IntValue);
					else if (this == Operation::BitXor)
						opReplace = Operand(ops[0].IntValue ^ ops[1].IntValue);
					return true;
				}
				return false;
			}
		};
		class ConversionOperation : public Operation
		{
		public:
			ConversionOperation(const wchar_t * name)
				:Operation(name)
			{}
			virtual bool TryEvaluate(Operand & opReplace, std::vector<Operand> & ops) override
			{
				if (ops[0].IsIntegral())
				{
					if (this == Operation::I2D)
						opReplace = Operand((double)ops[0].IntValue);
					else if (this == Operation::D2I)
						throw CompileErrorException(L"Invalid instruction d2i(int)");
					return true;
				}
				else if (ops[0].Type == OperandType::ConstDouble || ops[0].Type == OperandType::ConstFloat)
				{
					if (this == Operation::D2I)
					{
						if (ops[0].Type == OperandType::ConstDouble)
							opReplace = Operand((int)ops[0].DoubleValue);
						else
							opReplace = Operand((int)ops[0].FloatValue);
					}
					else if (this == Operation::I2D)
						throw CompileErrorException(L"Invalid instruction i2d(float)");
					return true;
				}
				return false;
			}
		};
		class SIncOperation : public Operation
		{
		public:
			SIncOperation(const wchar_t * name)
				:Operation(name)
			{}
			virtual bool TryEvaluate(Operand & opReplace, std::vector<Operand> & ops) override
			{
				if (ops[1].IsIntegral() && ops[2].IsIntegral())
				{
					ops[1].IntValue *= ops[2].IntValue;
					ops[2].IntValue = 1;
				}
				return false;
			}
		};

		AddSubOperation Operation_Add(L"add"), Operation_Sub(L"sub");
		FAddSubOperation Operation_FAdd(L"fadd"), Operation_FSub(L"fsub");
		MulDivOperation	Operation_Mul(L"mul"), Operation_Div(L"div");
		FMulDivOperation Operation_FMul(L"fmul"), Operation_FDiv(L"fdiv");
		ModOperation Operation_Mod(L"mod");
		ShiftOperation Operation_Lsh(L"lsh"), Operation_Rsh(L"rsh");
		CompareOperation Operation_Compare(L"compare"), Operation_Greater(L"greater"), Operation_Less(L"less"), Operation_Geq(L"geq"), 
				  Operation_Leq(L"leq"), Operation_Eql(L"eql"), Operation_Neq(L"neq");
		NegNotOperation Operation_Neg(L"neg"), Operation_Not(L"not"), Operation_BitNot(L"bitnot");
		AndOperation Operation_And(L"and");
		OrOperation Operation_Or(L"or"); 
		BitOperation Operation_BitAnd(L"bitand"), Operation_BitOr(L"bitor"), Operation_BitXor(L"bitxor");
		// Conversion
		ConversionOperation Operation_I2D(L"I2D"), Operation_D2I(L"D2I");
		SIncOperation Operation_SInc(L"sInc");
		// Memory
		Operation Operation_Lea(L"lea"); // address = lea(var)
		Operation Operation_Load(L"load"); // val = load(address, size)
		Operation Operation_Store(L"store"); // store(address, value, size)
		Operation Operation_Access(L"access"); // access(type, variable, offset)
		Operation Operation_Update(L"update"); // update(type, offset)
		// Control Flow
		Operation Operation_Call(L"call"); // call(FName, args)
		Operation Operation_Jump(L"jmp"), Operation_Branch(L"branch"); // branch(condition, address, branchType)
		Operation Operation_Ret(L"ret"); // ret(val)
		Operation Operation_Phi(L"phi");

		Operation::OperationPtr Operation::Add = &Operation_Add, 
								Operation::FAdd = &Operation_FAdd, 
								Operation::Sub = &Operation_Sub, 
								Operation::FSub = &Operation_FSub,
								Operation::Mul = &Operation_Mul, 
								Operation::FMul = &Operation_FMul, 
								Operation::Div = &Operation_Div, 
								Operation::FDiv = &Operation_FDiv, 
								Operation::Mod = &Operation_Mod;
		Operation::OperationPtr Operation::Lsh = &Operation_Lsh, 
								Operation::Rsh = &Operation_Rsh;
		Operation::OperationPtr Operation::Compare = &Operation_Compare,
								Operation::Greater = &Operation_Greater, 
								Operation::Less = &Operation_Less, 
								Operation::Geq = &Operation_Geq, 
								Operation::Leq = &Operation_Leq, 
								Operation::Eql = &Operation_Eql, 
								Operation::Neq = &Operation_Neq;
		Operation::OperationPtr Operation::Neg = &Operation_Neg, 
								Operation::Not = &Operation_Not, 
								Operation::And = &Operation_And, 
								Operation::Or = &Operation_Or, 
								Operation::BitNot = &Operation_BitNot,
								Operation::BitAnd = &Operation_BitAnd, 
								Operation::BitOr = &Operation_BitOr, 
								Operation::BitXor = &Operation_BitXor;
		// Conversion
		Operation::OperationPtr Operation::I2D = &Operation_I2D,
							    Operation::D2I = &Operation_D2I;
		// Memory
		Operation::OperationPtr Operation::SInc = &Operation_SInc;
		Operation::OperationPtr Operation::Lea = &Operation_Lea;
		Operation::OperationPtr Operation::Load = &Operation_Load;
		Operation::OperationPtr Operation::Store = &Operation_Store;
		Operation::OperationPtr Operation::Access = &Operation_Access;
		Operation::OperationPtr Operation::Update = &Operation_Update;
		// Control Flow
		Operation::OperationPtr Operation::Call = &Operation_Call;
		Operation::OperationPtr Operation::Jump = &Operation_Jump,
								Operation::Branch = &Operation_Branch;
		Operation::OperationPtr Operation::Ret = &Operation_Ret;
		Operation::OperationPtr Operation::Phi = &Operation_Phi;

		std::wstring Operand::ToString() const
		{
			std::wstring rs;
			switch (Type)
			{
			case Compiler::Intermediate::OperandType::None:
				rs += L"[none]";
				break;
			case Compiler::Intermediate::OperandType::ConstInt8:
				rs += ToWideString(IntValue);
				break;
			case Compiler::Intermediate::OperandType::ConstInt16:
				rs += ToWideString(IntValue);
				break;
			case Compiler::Intermediate::OperandType::ConstInt32:
				rs += ToWideString(IntValue);
				break;
			case Compiler::Intermediate::OperandType::ConstFloat:
				rs += ToWideString(FloatValue);
				break;
			case Compiler::Intermediate::OperandType::ConstDouble:
				rs += ToWideString(DoubleValue);
				break;
			case Compiler::Intermediate::OperandType::Variable:
				rs += Var->Name;
				rs += L"[";
				rs += ToWideString(Var->Id);
				rs += L"]";
				break;
			default:
				break;
			}
			return rs;
		}

		std::wstring Instruction::ToString() const
		{
			std::wstring rs;
			if (LeftOperand.Type != OperandType::None)
			{
				rs += LeftOperand.ToString();	//这个 LeftOperand 是 TryEvaluate 的第一个参数吗？
				//类型一样，等看看调用的地方有没有赋值吧.
				rs += L" = ";
			}
			if (Func)
				rs += Func->Name;//这个 Func 是操作符，三地址代码的. 
			rs += L"(";		//哦，他还有括号.
			for (size_t i = 0; i < Operands.size(); i++)
			{
				if (i > 0)
					rs += L", ";
				rs += Operands[i].ToString();	//参数之间以,隔开
			}
			rs += L")";
			return rs;
		}

		DataType Operand::GetDataType()		
		{
			switch (Type)
			{
			case OperandType::ConstDouble:
				return DataType::Double;
			case OperandType::ConstInt16:
			case OperandType::ConstInt8:
			case OperandType::ConstInt32:
				return DataType::Int;
			case OperandType::ConstFloat:
				return DataType::Float;
			default:
				return Var->Type;	//这个的意思是自定义变量的值类型？
			}
		}
	}
}
