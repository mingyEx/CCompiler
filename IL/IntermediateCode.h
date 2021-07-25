#ifndef COMPILER_INTERMEDIATE_CODE_H
#define COMPILER_INTERMEDIATE_CODE_H

#include "Basic.h"
#include "LibIO.h"
#include "CompileError.h"

namespace Compiler
{
	namespace Intermediate
	{
		using namespace CoreLib::Basic;
		using namespace CoreLib::IO;
		class InvalidProgramException : public Exception
		{
		public:
			InvalidProgramException()
			{}
			InvalidProgramException(const String & message)
				: Exception(message)
			{
			}

		};

		enum class OperandType
		{
			None,
			ConstInt8, ConstInt16, ConstInt32,
			ConstFloat, ConstDouble, 
			Variable
		};

		enum class DataType
		{
			Int, Float, Double
		};

		class Variable;

		class Operand	//操作数
		{
		public:
			OperandType Type;
			union
			{
				int IntValue;	//操作数的值
				double DoubleValue;
				float FloatValue;
				Variable * Var;	//指向变量的指针
				struct
				{
					int Values[2];
				};
			};
			String ToString() const;

			Operand()
			{
				Type = OperandType::None;
				IntValue = 0;
			}
			Operand(int val)
			{
				Type = OperandType::ConstInt32;
				IntValue = val;
			}
			Operand(float val)
			{
				Type = OperandType::ConstFloat;
				FloatValue = val;
			}
			Operand(double val)
			{
				Type = OperandType::ConstDouble;
				DoubleValue = val;
			}
			Operand(Variable * var)
			{
				Type = OperandType::Variable;
				Var = var;
			}
			DataType GetDataType();

			bool IsVariable()
			{
				return Type == OperandType::Variable;
			}
			bool IsIntegral()
			{
				return Type == OperandType::ConstInt16 ||
					   Type == OperandType::ConstInt32 ||
					   Type == OperandType::ConstInt8;
			}
		};

		
		class Operation	//操作符
		{
		public:
			const wchar_t * Name;
			Operation(const wchar_t * name)
				:Name(name)
			{}
			virtual bool TryEvaluate(Operand & opReplace, const List<Operand> & ops)	
				//猜测这里是操作符和操作数，为什么第一个却是 Operand ？ 这个就要去看函数体了.
				//因为第一个是返回值，后面一堆才是操作数，且操作符本身是用  Operation::Add 来实现的。
			{
				return false;
			}
			typedef Operation* OperationPtr;
			// Binary operations: F(op1, op2)

			//一些代表操作符 class 的指针
			static OperationPtr Add, FAdd, Sub, FSub, Mul, FMul, Div, FDiv, Mod;
			static OperationPtr Lsh, Rsh;
			static OperationPtr Compare, Greater, Less, Geq, Leq, Eql, Neq;
			static OperationPtr Neg, Not, And, Or, BitNot, BitAnd, BitOr, BitXor;
			
			// Conversion
			static OperationPtr I2D, D2I;
			// Scaled offset
			static OperationPtr SInc; // rs = base+index*scale
			// Memory
			static OperationPtr Lea;
			static OperationPtr Load; // val = load(address, type)
			static OperationPtr Store; // store(address, value, size)
			static OperationPtr Access; // access(type, variable, offset)
			static OperationPtr Update; // update(type, offset)
			// Control Flow
			static OperationPtr Call; // call(FName, args)
			static OperationPtr Jump, Branch; // branch(condition, address, branchType={jt,jf,jl,jle,jg,jge,je,jne})
			// jt,jf,jl,jle,jg,jge,je,jne 
			//对应的应该是:
			//jt: true 的时候跳转， jf : false 的时候跳转 
			//JL 前<后  Jump if  less
			//JLE 前<=后  Jump if  less or equal
			// JG 前>后   Jump if  greater
			// JGE 前>=后  Jump if  greater or equal
			// JE 前等于后  Jump if equal
			// JNE 前不等于后  Jump if not equal
			//但是branch指令，我的认识里，condition判断一下就只有两种结果了，他为啥要这么多?

			static OperationPtr Ret; // ret (val)
			static OperationPtr Phi;
		};

		inline int GetTypeSize(int id)
		{
			if (id == 1)
				return 4;
			else if (id == 2)
				return 4;
			else if (id == 3)
				return 8;
			else
				throw InvalidProgramException(L"Unkown type");
		}

		class ControlFlowNode;

		class Instruction
		{
		public:
			Operation * Func;			//嗯嗯,这里,单条指令以instruction来表示,所以这个Instruction就是一个三地址代码的抽象? 可是那就没必要list<操作数>了啊..
			Operand LeftOperand;

			ControlFlowNode * CFG_Node;	
			//节点指针,从当前块指向下一个节点,为了cfg与Instruction block一一对应的.
			List<Operand> Operands;		
			int Mark : 1;	//mark谁?
			int IsVolatile : 1;
			Instruction()
				: Func(0), CFG_Node(0), Mark(0), IsVolatile(0)
			{}
			Instruction(Instruction && instr)
				: Func(0), CFG_Node(0), Mark(0), IsVolatile(0)
			{
				operator = (_Move(instr));
			}
			Instruction(const Instruction& instr) = default;
			Instruction& operator = (const Instruction& instr) = default;
			Instruction & operator = (Instruction && instr)
			{
				Func = _Move(instr.Func);
				LeftOperand = _Move(instr.LeftOperand);
				Operands = _Move(instr.Operands);
				Mark = instr.Mark;
				return *this;
			}
			Instruction(Operation * f)
			{
				Func = f;
				LeftOperand = Operand();
			}
			Instruction(Operation * f, const Operand & op1)
			{
				Func = f;
				LeftOperand = Operand();
				Operands.Reserve(1);
				Operands.Add(op1);
			}
			Instruction(Operation * f, const Operand & op1, const Operand & op2)
			{
				Func = f;
				LeftOperand = Operand();
				Operands.Reserve(2);
				Operands.Add(op1);
				Operands.Add(op2);
			}
			Instruction(Operation * f, const Operand & op1, const Operand & op2, const Operand & op3)
			{
				Func = f;
				LeftOperand = Operand();
				Operands.Reserve(3);
				Operands.Add(op1);
				Operands.Add(op2);
				Operands.Add(op3);
			}
			Instruction(const Operand & leftOperand, Operation * f)
			{
				Func = f;
				LeftOperand = leftOperand;
			}
			Instruction(const Operand & leftOperand, Operation * f, const Operand & op1, const Operand & op2)
			{
				Func = f;
				LeftOperand = leftOperand;
				Operands.Reserve(2);
				Operands.Add(op1);
				Operands.Add(op2);
			}
			Instruction(const Operand & leftOperand, Operation * f, const Operand & op1, const Operand & op2, const Operand & op3)
			{
				Func = f;
				LeftOperand = leftOperand;
				Operands.Reserve(3);
				Operands.Add(op1);
				Operands.Add(op2);
				Operands.Add(op3);
			}
			Instruction(const Operand & leftOperand, Operation * f, const Operand & op1)
			{
				Func = f;
				LeftOperand = leftOperand;
				Operands.Reserve(1);
				Operands.Add(op1);
			}
			Instruction(const Operand & leftOperand, Operation * f,  const List<Operand> & ops)
			{
				Func = f;
				LeftOperand = leftOperand;
				Operands.Reserve(ops.Count());
				Operands.AddRange(ops);
			}
			Instruction(const Operand & leftOperand, Operation * f, List<Operand> && ops)
			{
				Func = f;
				LeftOperand = leftOperand;
				Operands = _Move(ops);
			}
			String ToString() const;
		};

		typedef LinkedNode<Instruction> InstructionNode;

		enum class MemoryLocationType
		{
			Register, Stack
		};

		//这个内存位置，里面有一个stack,不懂。联系上面的栈与寄存器，所以这个应该是运行时表示相关。
		//但是为什么里面没有寄存器呢..
		//todo 栈是用来辅助其他的,并非如此,待整理;
		class MemoryLocation
		{
		public:
			MemoryLocationType Type;
			int Value;
			MemoryLocation()
			{
				Type = MemoryLocationType::Stack;
				Value = 0;
			}
			MemoryLocation(MemoryLocationType type, int value)
			{
				Type = type;
				Value = value;
			}
			
			bool operator == (const MemoryLocation loc)
			{
				return Type == loc.Type && Value == loc.Value;
			}
			bool operator != (const MemoryLocation loc)
			{
				return Type != loc.Type || Value != loc.Value;
			}
		};

		class Variable
		{
		public:
			int Id;	//变量在表里的位置
			String Name;
			int Version;// 第几次修改的版本.
			DataType Type;
			int Size; // in bytes
			MemoryLocation Location;
			Variable()	//默认int的实现
			{
				Version = 0;
				Size = 0;
				Type = DataType::Int;
			}
			Variable(const Variable & var)
			{
				Id = var.Id;
				Name = var.Name;
				Version = var.Version;
				Type = var.Type;
				Size = var.Size;
				Location = var.Location;
			}
			Variable(Variable && var)
			{
				operator=(_Move(var));
			}
			Variable & operator = (Variable && var)
			{
				Id = var.Id;
				Name = _Move(var.Name);
				Version = var.Version;
				Size = var.Size;
				Type = var.Type;
				return *this;
			}
			Variable(const String & name)
			{
				Name = name;
				Version = 0;
				Size = 4;
				Type = DataType::Int;
			}
			Variable(const String & name, int sizeInBytes)
			{
				Name = name;
				Version = 0;
				Size = sizeInBytes;
				Type = DataType::Int;
			}
			Variable(const String & name, int sizeInBytes, int version)
			{
				Name = name;
				Version = version;
				Size = sizeInBytes;
				Type = DataType::Int;
			}
		};

		class Function
		{
		public:
			String Name;
			LinkedList<Instruction> Instructions;	
			//函数是由一系列指令形成的，而非cst里的几条语句组成的树。cst里，函数是节点连起来的，没有"翻译到元语言的call操作"，只有名，参，体的树节点而已。

			List<unsigned char> Code;			//这里的Code 是保存了那些指令吗？
			List<RefPtr<Variable>> Variables; // do not add directly	这个不是形参，那该是什么？先挂起
			List<RefPtr<Variable>> Parameters; // do not add directly	形参
			int ParameterSize;
			int VariableSize;
			Function()
			{}
			Function(Function && func)
			{
				operator=(_Move(func));
			}
			Function & operator = (Function && func)
			{
				Name = _Move(func.Name);
				Instructions = _Move(func.Instructions);
				Variables = _Move(func.Variables);
				Parameters = _Move(func.Parameters);
				Code = _Move(func.Code);
				return *this;
			}
			Variable * AddVariable(const String& name, int version=0, int size=4)
			{
				Variable * rs = new Variable(name, size, version);
				rs->Id = Parameters.Count() + Variables.Count();
				Variables.Add(rs);
				return rs;
			}
			Variable * AddParameter(const String& name, int version=0, int size=4)
			{
				if (Variables.Count())
					throw InvalidOperationException(L"Parameter cannot be inserted after variables.");
				//形参不能放到变量之后是什么鬼.. 难道是默认参数？ 不是，可能只是指IR里，形参在前才好计算吧。
				Variable * rs = new Variable(name, size, version);
				rs->Id = Parameters.Count() + Variables.Count();
				Parameters.Add(rs);
				return rs;
			}
			void Dump(TextWriter & writer)	//把IR指令写入到文件里,给CFGView用
			{
				writer<<L"Function "<<Name<<EndLine;
				int i = 0;
				for (auto & instr:Instructions)
				{
					writer<<i<<L'\t'<<instr.ToString()<<EndLine;
					i++;
				}
				writer<<L"End Function"<<EndLine;
			}
		};

		class Program
		{
		public:
			List<Function> Functions;	//程序就是一堆函数
			Program()
			{}
			Program(Program && code)
			{
				operator=(_Move(code));
			}
			Program & operator = (Program && code)
			{
				Functions = _Move(code.Functions);
				return *this;
			}
			void Dump(const String & fileName)
			{
				StreamWriter writer(fileName);
				//调用的func的dump来吧指令等写入文本.
				for (auto & func:Functions)
					func.Dump(writer);
			}
		};
	}
}

#endif