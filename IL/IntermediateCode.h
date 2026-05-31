#ifndef COMPILER_INTERMEDIATE_CODE_H
#define COMPILER_INTERMEDIATE_CODE_H

#include <filesystem>
#include <fstream>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "CompileError.h"

namespace Compiler
{
	namespace Intermediate
	{
		class InvalidProgramException : public std::runtime_error
		{
		public:
			InvalidProgramException()
				: std::runtime_error("")
			{}
			explicit InvalidProgramException(const char* message)
				: std::runtime_error(message)
			{
			}
			explicit InvalidProgramException(const std::string& message)
				: std::runtime_error(message)
			{
			}
			explicit InvalidProgramException(const wchar_t* message)
				: std::runtime_error(ToNarrow(std::wstring(message)))
			{
			}
			explicit InvalidProgramException(const std::wstring& message)
				: std::runtime_error(ToNarrow(message))
			{
			}

		private:
			static std::string ToNarrow(const std::wstring& message)
			{
				std::string result;
				result.reserve(message.size());
				for (const wchar_t ch : message)
					result.push_back(static_cast<char>(ch));
				return result;
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
			std::wstring ToString() const;

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
			virtual bool TryEvaluate(Operand & opReplace, std::vector<Operand> & ops)	
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

		//单条指令以instruction来表示,所以这个Instruction就是一个三地址代码的抽象
		class Instruction
		{
		public:
			Operation * Func;		//操作符，比如move,add之类的。		
			Operand LeftOperand;
			ControlFlowNode * CFG_Node;	
			//节点指针,从当前块指向下一个节点,为了cfg与Instruction block一一对应的.
			std::vector<Operand> Operands;//一条指令对应多个操作数
			int Mark : 1;	//标记关键节点的flag,是优化部分用到的。
			int IsVolatile : 1;
			Instruction()
				: Func(0), CFG_Node(0), Mark(0), IsVolatile(0)
			{}
			Instruction(Instruction && instr)
				: Func(0), CFG_Node(0), Mark(0), IsVolatile(0)
			{
				operator = (std::move(instr));
			}
			Instruction(const Instruction& instr) = default;
			Instruction& operator = (const Instruction& instr) = default;
			Instruction & operator = (Instruction && instr)
			{
				Func = std::move(instr.Func);
				LeftOperand = std::move(instr.LeftOperand);
				CFG_Node = instr.CFG_Node;
				Operands = std::move(instr.Operands);
				Mark = instr.Mark;
				IsVolatile = instr.IsVolatile;
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
				Operands.reserve(1);
				Operands.push_back(op1);
			}
			Instruction(Operation * f, const Operand & op1, const Operand & op2)
			{
				Func = f;
				LeftOperand = Operand();
				Operands.reserve(2);
				Operands.push_back(op1);
				Operands.push_back(op2);
			}
			Instruction(Operation * f, const Operand & op1, const Operand & op2, const Operand & op3)
			{
				Func = f;
				LeftOperand = Operand();
				Operands.reserve(3);
				Operands.push_back(op1);
				Operands.push_back(op2);
				Operands.push_back(op3);
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
				Operands.reserve(2);
				Operands.push_back(op1);
				Operands.push_back(op2);
			}
			Instruction(const Operand & leftOperand, Operation * f, const Operand & op1, const Operand & op2, const Operand & op3)
			{
				Func = f;
				LeftOperand = leftOperand;
				Operands.reserve(3);
				Operands.push_back(op1);
				Operands.push_back(op2);
				Operands.push_back(op3);
			}
			Instruction(const Operand & leftOperand, Operation * f, const Operand & op1)
			{
				Func = f;
				LeftOperand = leftOperand;
				Operands.reserve(1);
				Operands.push_back(op1);
			}
			Instruction(const Operand & leftOperand, Operation * f, const std::vector<Operand> & ops)
			{
				Func = f;
				LeftOperand = leftOperand;
				Operands = ops;
			}
			Instruction(const Operand & leftOperand, Operation * f, std::vector<Operand> && ops)
			{
				Func = f;
				LeftOperand = leftOperand;
				Operands = std::move(ops);
			}
			std::wstring ToString() const;
		};

		class InstructionList;

		class InstructionNode
		{
			friend class InstructionList;
		private:
			InstructionList* owner = nullptr;
		public:
			Instruction Value;

			InstructionNode() = default;
			explicit InstructionNode(const Instruction& instruction)
				: Value(instruction)
			{
			}

			InstructionNode* GetPrevious();
			InstructionNode* GetNext();
			InstructionNode* InsertBefore(const Instruction& instruction);
			InstructionNode* InsertAfter(const Instruction& instruction);
			void Delete();
		};

		class InstructionList
		{
		private:
			std::list<InstructionNode> nodes;

			std::list<InstructionNode>::iterator FindNode(InstructionNode* node)
			{
				for (auto iter = nodes.begin(); iter != nodes.end(); ++iter)
				{
					if (&*iter == node)
						return iter;
				}
				return nodes.end();
			}

			std::list<InstructionNode>::const_iterator FindNode(const InstructionNode* node) const
			{
				for (auto iter = nodes.begin(); iter != nodes.end(); ++iter)
				{
					if (&*iter == node)
						return iter;
				}
				return nodes.end();
			}

			void ResetOwners()
			{
				for (auto& node : nodes)
					node.owner = this;
			}

		public:
			class Iterator
			{
			private:
				std::list<InstructionNode>::iterator iter;
			public:
				explicit Iterator(std::list<InstructionNode>::iterator iterator)
					: iter(iterator)
				{
				}

				Instruction& operator*() const
				{
					return iter->Value;
				}

				Iterator& operator++()
				{
					++iter;
					return *this;
				}

				bool operator!=(const Iterator& other) const
				{
					return iter != other.iter;
				}
			};

			class ConstIterator
			{
			private:
				std::list<InstructionNode>::const_iterator iter;
			public:
				explicit ConstIterator(std::list<InstructionNode>::const_iterator iterator)
					: iter(iterator)
				{
				}

				const Instruction& operator*() const
				{
					return iter->Value;
				}

				ConstIterator& operator++()
				{
					++iter;
					return *this;
				}

				bool operator!=(const ConstIterator& other) const
				{
					return iter != other.iter;
				}
			};

			InstructionList() = default;

			InstructionList(const InstructionList& other)
			{
				for (const auto& instruction : other)
					AddLast(instruction);
			}

			InstructionList(InstructionList&& other) noexcept
				: nodes(std::move(other.nodes))
			{
				ResetOwners();
			}

			InstructionList& operator=(const InstructionList& other)
			{
				if (this == &other)
					return *this;
				Clear();
				for (const auto& instruction : other)
					AddLast(instruction);
				return *this;
			}

			InstructionList& operator=(InstructionList&& other) noexcept
			{
				if (this == &other)
					return *this;
				nodes = std::move(other.nodes);
				ResetOwners();
				return *this;
			}

			Iterator begin()
			{
				return Iterator(nodes.begin());
			}

			Iterator end()
			{
				return Iterator(nodes.end());
			}

			ConstIterator begin() const
			{
				return ConstIterator(nodes.begin());
			}

			ConstIterator end() const
			{
				return ConstIterator(nodes.end());
			}

			template<typename IteratorFunc>
			void ForEach(const IteratorFunc& func)
			{
				for (auto& node : nodes)
					func(node.Value);
			}

			InstructionNode* FirstNode()
			{
				return nodes.empty() ? nullptr : &nodes.front();
			}

			InstructionNode* LastNode()
			{
				return nodes.empty() ? nullptr : &nodes.back();
			}

			Instruction& First()
			{
				if (nodes.empty())
					throw std::logic_error("InstructionList is empty.");
				return nodes.front().Value;
			}

			Instruction& Last()
			{
				if (nodes.empty())
					throw std::logic_error("InstructionList is empty.");
				return nodes.back().Value;
			}

			InstructionNode* AddFirst(const Instruction& instruction)
			{
				nodes.emplace_front(instruction);
				nodes.front().owner = this;
				return &nodes.front();
			}

			InstructionNode* AddLast(const Instruction& instruction)
			{
				nodes.emplace_back(instruction);
				nodes.back().owner = this;
				return &nodes.back();
			}

			void Delete(InstructionNode* node)
			{
				auto iter = FindNode(node);
				if (iter != nodes.end())
					nodes.erase(iter);
			}

			void Clear()
			{
				nodes.clear();
			}

			int Count() const
			{
				return static_cast<int>(nodes.size());
			}

			std::vector<Instruction> ToVector() const
			{
				std::vector<Instruction> result;
				result.reserve(nodes.size());
				for (const auto& node : nodes)
					result.push_back(node.Value);
				return result;
			}

			InstructionNode* PreviousNode(InstructionNode* node)
			{
				auto iter = FindNode(node);
				if (iter == nodes.end() || iter == nodes.begin())
					return nullptr;
				--iter;
				return &*iter;
			}

			InstructionNode* NextNode(InstructionNode* node)
			{
				auto iter = FindNode(node);
				if (iter == nodes.end())
					return nullptr;
				++iter;
				return iter == nodes.end() ? nullptr : &*iter;
			}

			InstructionNode* InsertBefore(InstructionNode* node, const Instruction& instruction)
			{
				auto iter = FindNode(node);
				if (iter == nodes.end())
					return nullptr;
				auto inserted = nodes.emplace(iter, instruction);
				inserted->owner = this;
				return &*inserted;
			}

			InstructionNode* InsertAfter(InstructionNode* node, const Instruction& instruction)
			{
				auto iter = FindNode(node);
				if (iter == nodes.end())
					return nullptr;
				++iter;
				auto inserted = nodes.emplace(iter, instruction);
				inserted->owner = this;
				return &*inserted;
			}
		};

		inline InstructionNode* InstructionNode::GetPrevious()
		{
			return owner ? owner->PreviousNode(this) : nullptr;
		}

		inline InstructionNode* InstructionNode::GetNext()
		{
			return owner ? owner->NextNode(this) : nullptr;
		}

		inline InstructionNode* InstructionNode::InsertBefore(const Instruction& instruction)
		{
			return owner ? owner->InsertBefore(this, instruction) : nullptr;
		}

		inline InstructionNode* InstructionNode::InsertAfter(const Instruction& instruction)
		{
			return owner ? owner->InsertAfter(this, instruction) : nullptr;
		}

		inline void InstructionNode::Delete()
		{
			if (owner)
				owner->Delete(this);
		}

		enum class MemoryLocationType
		{
			Register, Stack	//标记变量的位置在stack还是寄存器。
		};

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
			std::wstring Name;
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
				operator=(std::move(var));
			}
			Variable & operator = (Variable && var)
			{
				Id = var.Id;
				Name = std::move(var.Name);
				Version = var.Version;
				Size = var.Size;
				Type = var.Type;
				Location = var.Location;
				return *this;
			}
			Variable(const std::wstring & name)
			{
				Name = name;
				Version = 0;
				Size = 4;
				Type = DataType::Int;
			}
			Variable(const std::wstring & name, int sizeInBytes)
			{
				Name = name;
				Version = 0;
				Size = sizeInBytes;
				Type = DataType::Int;
			}
			Variable(const std::wstring & name, int sizeInBytes, int version)
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
			std::wstring Name;
			InstructionList Instructions;	
			//函数是由里面一系列表达式翻译成的一系列指令形成的，
			//而非cst里的几条语句组成的树。cst里，函数是节点连起来的，没有"翻译到元语言的call操作"，只有名，参，体的树节点而已。

			std::vector<std::shared_ptr<Variable>> Variables; // do not add directly	实参？
			std::vector<std::shared_ptr<Variable>> Parameters; // do not add directly	形参
			int ParameterSize;
			int VariableSize;
			Function()
			{}
			Function(Function && func)
			{
				operator=(std::move(func));
			}
			Function & operator = (Function && func)
			{
				Name = std::move(func.Name);
				Instructions = std::move(func.Instructions);
				Variables = std::move(func.Variables);
				Parameters = std::move(func.Parameters);
				ParameterSize = func.ParameterSize;
				VariableSize = func.VariableSize;
				return *this;
			}
			void SetName(const std::wstring& name)
			{
				Name = name;
			}
			Variable * AddVariable(const std::wstring& name, int version=0, int size=4)
			{
				auto rs = std::make_shared<Variable>(name, size, version);
				rs->Id = static_cast<int>(Parameters.size() + Variables.size());
				Variables.push_back(rs);
				return rs.get();
			}
			Variable * AddParameter(const std::wstring& name, int version=0, int size=4)
			{
				if (!Variables.empty())
					throw std::logic_error("Parameter cannot be inserted after variables.");
				//形参不能放到变量之后是什么鬼.. 难道是默认参数？ 不是，可能只是指IR里，形参在前才好计算吧。
				auto rs = std::make_shared<Variable>(name, size, version);
				rs->Id = static_cast<int>(Parameters.size() + Variables.size());
				Parameters.push_back(rs);
				return rs.get();
			}
			void Dump(std::wostream& writer)	//把IR指令写入到文件里,给CFGView用
			{
				writer << L"Function " << Name.c_str() << L'\n';
				int i = 0;
				for (auto & instr:Instructions)
				{
					writer << i << L'\t' << instr.ToString() << L'\n';
					i++;
				}
				writer << L"End Function" << L'\n';
			}
		};

		class Program
		{
		public:
			std::vector<Function> Functions;	//程序就是一堆函数
			Program()
			{}
			Program(Program && code)
			{
				operator=(std::move(code));
			}
			Program & operator = (Program && code)
			{
				Functions = std::move(code.Functions);
				return *this;
			}
			void Dump(const std::filesystem::path & fileName)
			{
				std::wofstream writer(fileName);
				//调用func的dump来把指令等写入文本给cfgView显示.
				for (auto & func:Functions)
					func.Dump(writer);
			}
		};
	}
}

#endif
