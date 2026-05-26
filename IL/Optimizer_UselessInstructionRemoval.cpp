#include "Optimization.h"
#include "Exception.h"
#include "ScopeDictionary.h"
#include <sstream>

namespace Compiler
{
	namespace Intermediate
	{
		// This optimizer perform following transformations:
		// 1. Remove instructions that involve identities, e.g. replace
		//	  a = b + 1 with a = b		
		// 2. Remove useless phi functions, e.g. a[1] = phi(a[0], a[0])
		// 3. Fold constants, e.g. a = 1 + 2 => a = 3
		//可以参见 6.14 Removing Useless φ-instructions
		// http://coins-compiler.osdn.jp/international/ssa/ssa-external-english.pdf  反正我是没看懂 =_=
		class UselessInstructionOptimizer : public IntraProcOptimizer
		{
		private:
			std::wstring ToHex(int value)
			{
				std::wostringstream writer;
				writer << std::hex << value;
				return writer.str();
			}

			std::wstring GetExprString(Variable * var)	//获得表达式的字符串形式？ 这里传入的是变量，虽然名为Expr，但是没有递归，所以应该是最基本的形式.
			{
				return L"#" + ToHex(var->Id);
			}
			void AppendOperandExprString(std::wstring & text, Operand & op)	//操作数
			{
				if (op.IsIntegral() || op.Type == OperandType::ConstFloat)	//后面的是进制 
					text += ToHex(op.IntValue);
				else if (op.Type == OperandType::ConstDouble)	//哦，那么0,1分别是整数部分和小数部分吗，为什么float不用呢?
				{
					text += ToHex(op.Values[0]);	//float占4个字节32位，double占8个字节64位	所以需要float的两倍，完结！
					text += ToHex(op.Values[1]);
				}	//https://stackoverflow.com/a/49991136/13792395 表示方法我知道，我想要知道的是，如何保存的?在IR层次上。
				
				else if (op.IsVariable())	//哦，这里是作为操作数的变量，跟GetExprString里不一样。
				{
					text += L'#';
					text += ToHex(op.Var->Id);		//这个Id是啥？
				}
				else
					throw InvalidProgramException(L"Unknown operand type.");
			}
			std::wstring GetExprString(Instruction & instr)	//对指令的重载版本。
			{
				std::wstring text;
				if (instr.Func)
				{
					text += instr.Func->Name;	//这个应该是上面那个标号？ 或者是 我觉得应该是操作符？左右操作数都有了，好吧，暂定是它.
					text += L':';				//可是没有任何地方出现了这个: 啊...
				}
				int i = 0;
				for (auto& op : instr.Operands)
				{
					if (i>0)
						text += L',';		//func跟参数之间只以 , 分割.. 那里用到这种格式了？
					AppendOperandExprString(text, op);	
				}
				return text;
			}
			bool TraverseNode(ControlFlowGraph *program, ControlFlowNode * node, ScopeDictionary<std::wstring, Operand> & exprMap)	//遍历节点
			{
				bool changed = false;
				if (node == program->Sink)
					return false;
				exprMap.PushScope();	//实现作用域词典的栈.
				if (node != program->Source)	//从开头开始.
				{
					for (auto instrNode = FirstInstructionNode(node->Code); instrNode != nullptr; )	//遍历当前节点里list<Instruction>里的每一个.
					{
						auto nextInstrNode = NextInstructionNode(instrNode);
						Instruction & instr = GetInstruction(instrNode);
						// Process Phi functions
						if (instr.Func == Operation::Phi)
						{
							Variable * v = instr.Operands[0].Var;	//第一个操作数的变量值的指针赋值出来
							bool sameArg = true;					//?
							for (auto op : instr.Operands)			//遍历Phi指令的所有的操作数，若其他操作数与第一个不相等也不等于结果则设置为false.
							{										//看起来sameArg 无论如何都会被删掉..
								if (op.Var != v && op.Var != instr.LeftOperand.Var)//第一个操作数，左边一定会相等，所以else来自于右边的不相等。剩下的来自于每一个都跟第一个不一样，且跟左边不一样，可这无法保证第二个跟第三个不一样啊卧槽！ 首先，先考虑作者给出的情况，a[1] = phi(a[0], a[0])，对这个来说，确实只要保证这个就行了，别的先不管，我们跟着他的逻辑走，暂时不考虑其正确性，因为我们连输入是啥都他妈的不知道呢。
								{
									sameArg = false;
									break;
								}
							}
							if (sameArg)	//Operands跟第一个不一样，或者... 算了先找下逻辑. (a!=b && a!=c)  的取反是 a==b || a==c ,所以这里代表的是，有参数跟a[1]一样，或者 a[0] a[0] 这种情况，这时候就？
							{
								Operand valueOp;
								/*if (!exprMap.TryGetValue(GetExprString(v), valueOp))
									throw InvalidProgramException(L"Value entry not found in exprMap");*/

								exprMap.Add(GetExprString(instr.LeftOperand.Var), instr.Operands[0]);	//把左操作数和第一个操作数都组成一对塞到作用域词典里?
								RemoveInstruction(instrNode);			//为什么呢？因为取消掉phi之后就变成了简单的赋值，不对，是“只有这一个操作数”。？嗯，左边是名字，右边是值?
								//对 Instruction 来说，它的Operands 要么是值，要么是指向变量的指针，所以这么搞没有问题啦！
								changed = true;	//都塞进去了，为什么还要设定为true呢？
								instrNode = nextInstrNode;
								continue;
							}
							else	//这个针对的是phi不可被删除的情况,亦即：操作符是phi,但是phi(x1,x2)相等，或者有一个等于phi结果 的情况。 这种时候不满足
							{		//  a[1] = phi(a[0], a[0]) 啊.. 哦我知道了，只要有哪怕一点，两个不等于一个的情况，都应该删掉phi ? 这很奇怪，如果五个参数有三个是怎么办?
								std::wstring exprString = GetExprString(instr);//instr 是当前Node 的第一条指令.
								Operand opValue;	//作为传出参数?
								if (exprMap.TryGetValueInCurrentScope(exprString, opValue))//opValue 是out 参数. 这里的意思是，如果此参数存在，就如此
								{
									exprMap.Add(GetExprString(instr.LeftOperand.Var), opValue);		//然后把他以及对应的值加到作用域字典里去.
									RemoveInstruction(instrNode);	//简单赋值之后，把当前指令删除掉 ...    为什么？ 不该是仅仅在phi可以被优化的时候才删掉吗?
									changed = true;
									instrNode = nextInstrNode;
									continue;
								}
								else
								{
									exprMap.Add(exprString, instr.LeftOperand); 
								}	
								//如果当前作用域里该变量不存在，就添加一个映射。 添加的到底是什么...//我们知道phi就是简单赋值，所以这里全都吧phi删掉的话？ 其实删掉的是...  只要phi就全部都定值吗？
							}//这个分支里所有的子分支都对应着phi不可被删除的情况，if else都对
						}	//这个分支都对应着第一条是phi指令的情况，可是为什么每个都指向删掉本条指令？

						else
						{
							// replace operands
							for (auto & operand : instr.Operands)
							{
								Operand op;
								if (operand.IsVariable()&&exprMap.TryGetValue(GetExprString(operand.Var), op))
								{
									operand = op;
									changed = true;
								}
							}
							// try evaluate
							Operand opReplace;
							if (instr.Func && instr.Func->TryEvaluate(opReplace, instr.Operands))
							{
								exprMap.Add(GetExprString(instr.LeftOperand.Var), opReplace);
								changed = true;
								instr.Func = 0;
								instr.Operands.clear();
								instr.Operands.push_back(opReplace);
							}
							else if (instr.Operands.size() == 1 && !instr.Func)
							{
								if (instr.LeftOperand.IsVariable())
									exprMap.Add(GetExprString(instr.LeftOperand.Var), instr.Operands[0]);
								instrNode = nextInstrNode;
								continue;
							}
							// try match with existing values
							std::wstring exprString = GetExprString(instr);
							Operand opValue;
							bool isConst = instr.Operands.size() == 1 && instr.Func == 0 &&
								!instr.Operands[0].IsVariable();
							if (!isConst && exprMap.TryGetValue(exprString, opValue) && !instr.IsVolatile)
							{
								exprMap.Add(GetExprString(instr.LeftOperand.Var), opValue);
								changed = true;
								program->VarDefs[instr.LeftOperand.Var->Id] = 0;
								RemoveInstruction(instrNode);
								instrNode = nextInstrNode;
								continue;
							}
							// define a new value
							if (instr.LeftOperand.IsVariable())
								exprMap.Add(exprString, instr.LeftOperand);
							else if (instr.Func == Operation::Store && !instr.IsVolatile)
							{
								Instruction loadInstr(Operation::Load, instr.Operands[0], instr.Operands[2]);
								exprMap.Add(GetExprString(loadInstr), instr.Operands[1]);
							}
						}
						instrNode = nextInstrNode;
					}
					// Modify phi function arguments of CFG successors
					for (auto childNode : node->Exits)
					{
						if (!childNode)
							break;
						for (auto & instr : childNode->Code) 
							if (instr.Func == Operation::Phi)
								for (auto & op : instr.Operands)
								{
									Operand opReplace;
									if (exprMap.TryGetValue(GetExprString(op.Var), opReplace)
										&& opReplace.IsVariable())
									{
										op = opReplace;
										changed = true;
									}
								}
					}
				}
				// recur in DomChildren  
				//循环/递归 在DOM 孩子上？ 我已经忘记了孩子是啥了...

				//这个是本函数里继续往下的部分.
				bool nodeIsBranch = node->GetExitCount()==2 && node->Code.Count()>0 && node->Code.Last().Func == Operation::Branch && node->Code.Last().Operands[0].IsVariable();
				//人话版，有两个退出节点，指令数量很多，最后一个的指令是分支，操作数是变量。 这种形式。 可是为啥操作数的目标要是变量？其实是根据第一个值来判断后面的.
				for (auto & childNode : node->DomChildren)	//就是支配者树的孩子节点!
				{
					// propagate branch specific variables
					//传播分支特定变量
					bool shouldPopScope = false;
					if (nodeIsBranch)
					{
						auto branchVar = node->Code.Last().Operands[0].Var;	//因为这个是变量
						int branchType = node->Code.Last().Operands[2].IntValue;	//而这个是值,所以. 后面的东西不一样.
						
						int value = 0;
						if (branchType > 1)	//我所见到的，第二个也都是1.
							throw InvalidProgramException(L"unknown branch type. This optimizer only recognizes JT and JF.");
						if (childNode == node->Exits[0])	//就是IDOM树的子节点.
						{
							if (branchType == 0) // jtrue 0			说明什么？ 我只见过1....那就当时false==1 吧，就是说，如果为0则value设置为1. 干嘛用的?
								value = 1;
							else
								value = 0;
							shouldPopScope = true;
							exprMap.PushScope();
							exprMap.Add(GetExprString(branchVar), value);
						}
						else if (childNode == node->Exits[1])	//如果为1，则是第二个退出节点， 别的倒是都一样emmm .
						{
							if (branchType == 0) // jtrue
								value = 0;
							else
								value = 1;
							shouldPopScope = true;
							exprMap.PushScope();
							exprMap.Add(GetExprString(branchVar), value);
						}
					}
					//如果是分支就如此处理，我也不知道在干嘛.
					//下面就是处理结果跟changed逐位或，得到的结果是..? 不知道。
					changed |= TraverseNode(program, childNode, exprMap);
					if (shouldPopScope)
						exprMap.PopScope();
				}
				exprMap.PopScope();
				return changed;
			}	//代码看完了，不知道在干嘛，只知道遍历了一下节点，然后加了点名字之类的，不管了 =_=||
		public:
			virtual ProgramOptimizationResult Optimize(std::shared_ptr<ControlFlowGraph> program) override
			{
				ProgramOptimizationResult rs;
				ScopeDictionary<std::wstring, Operand> exprMap;
				rs.Changed = TraverseNode(program.get(), program->Source, exprMap);
				rs.Program = program;
				return rs;
			}
		};
		std::unique_ptr<IntraProcOptimizer> CreateUselessInstructionOptimizer()
		{
			return std::make_unique<UselessInstructionOptimizer>();
		}
	}
}
