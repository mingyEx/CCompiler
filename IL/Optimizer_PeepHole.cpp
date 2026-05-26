#include "Optimization.h"
#include "TransformInvariant.h"

namespace Compiler
{
	namespace Intermediate
	{
		//我还记得老师说.. 算了啥都没说.
		//就是几个指令重写操作，不管了.
		//我得对这几个指令更清楚，并且搞懂800000 到底是干嘛的，才能看清楚这段.
		//来完成它吧！
		
		//留下两个问题 1.为什么要 IntValue-- 2.IntValue |= (int)0x80000000是在干鸡巴啥?
		class PeepHoleOptimizer : public IntraProcOptimizer
		{
		private:
			static bool IsPowerOfTwo(int val)
			{
				return val > 1 && (val & (val-1)) == 0;	//又是位运算，我想知道为什么，搜到的都是什么傻逼..
				//带入了几个数字，发现“就是有这样的性质”，我能怎么办，我他妈个逼的怎么知道.. 
				//如果你们计算机专业里到处都是这种莫名其妙的东西，而且多到数不清，我他妈的哪辈子学的完.
			}
			//这个是，求2的对数.
			static int Log2(int val)
			{
				int x=0;  
				while(val>1)  
				{  
					val>>=1;  
					x++;  
				}  
				return x;  
			}
		public:
			virtual ProgramOptimizationResult Optimize(std::shared_ptr<ControlFlowGraph> program) override
			{
				ProgramOptimizationResult result;
				result.Program = program;
				result.Changed = false;
				for (auto & node : program->Nodes)	//对每个节点
				{
					for (auto instrNode = FirstInstructionNode(node->Code); instrNode != nullptr; instrNode = NextInstructionNode(instrNode))	//里面的每条指令
					{
						auto & instr = GetInstruction(instrNode);	//里面的值
						//一堆if 是在干嘛?

						//如果操作指令是mod并且第一个是整数，就把此指令重写成位操作
						//mod操作数的意义是，求余数？ 是的，这里是，如果mod操作数的参数是2的n次方，就直接计算log2.
						if (instr.Func == Operation::Mod && instr.Operands[1].IsIntegral() &&
							IsPowerOfTwo(instr.Operands[1].IntValue))
						{
							instr.Func = Operation::BitAnd;
							instr.Operands[1].IntValue--;	//首先自减	为什么要自减? 比如100000，是2的次方，减一变成111111 这样接下来的运算才能进行呀. 或者这个mod本来就是计算2的？ 不然下面为啥要有一个if?
							if (instr.Operands[1].GetDataType() == DataType::Int) // signed
								instr.Operands[1].IntValue |= (int)0x80000000;//然后，这个按位与是什么？这个数字有特殊含义吗? 为 -2^31 
							//按位与 为什么要这么做? 将var的最高位置一. 有什么用？
							//我猜它的用途是，回绕？
						}

						//对除法的操作.
						if (instr.Func == Operation::Div && instr.Operands[1].IsIntegral() &&
							IsPowerOfTwo(instr.Operands[1].IntValue))
						{
							instr.Func = Operation::Rsh;
							instr.Operands[1].IntValue = Log2(instr.Operands[1].IntValue);
						}
						if (instr.Func == Operation::Mul && instr.Operands[1].IsIntegral() &&
							IsPowerOfTwo(instr.Operands[1].IntValue))
						{
							instr.Func = Operation::Lsh;
							instr.Operands[1].IntValue = Log2(instr.Operands[1].IntValue);
						}
					}
				}
				return result;
			}
		};

		std::unique_ptr<IntraProcOptimizer> CreatePeepHoleOptimizer()
		{
			return std::make_unique<PeepHoleOptimizer>();
		}
	}
}
