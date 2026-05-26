#include "Optimization.h"
#include "InterferenceAnalysis.h"
#include "ScopeDictionary.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Compiler
{
	//更近似 https://www.clear.rice.edu/comp512/Lectures/13SSA-2.pdf 而不是DFA书上的那个计算交集的方式.
	namespace Intermediate
	{
		Operation opParallelCopy(L"ParallelCopy");	
		static Operation * ParallelCopy = &opParallelCopy;

		class OutOfSSA_Transform : public IntraProcOptimizer
		{
		private:
			std::vector<std::unique_ptr<Variable>> temporaryVariableOwners;

			Variable* MakeParallelCopyTemp(Variable* source)
			{
				auto owner = std::make_unique<Variable>(*source);
				auto temp = owner.get();
				temp->Name = L"^tmpcpy_" + std::to_wstring(static_cast<int>(source->Type));
				temp->Version = 0;
				temporaryVariableOwners.push_back(std::move(owner));
				return temp;
			}

			//以逆后续方式遍历整个程序，对每条指令,仅操作符为phi的时候，执行函数func.
			template <typename F>
			void ForEachPhi(ControlFlowGraph * program, F func)
			{
				auto traverse = program->GetPostOrderOnReverseCFG(); 
				for (int i = static_cast<int>(traverse.size()) - 1; i>=0; i--)
				{
					auto node = traverse[i];
					for (auto instrNode = FirstInstructionNode(node->Code); instrNode; instrNode = NextInstructionNode(instrNode))
					{
						auto & instr = GetInstruction(instrNode);
						if (instr.Func != Operation::Phi) break;
						func(instrNode, instr);
					}
				}
			}
			
			//遍历所有指令，为ParallelCopy 的时候，执行函数F.
			template <typename F>
			void ForEachParallelCopy(ControlFlowGraph * program, F func)
			{
				for (int i = 0; i < static_cast<int>(program->Nodes.size()); i++)
				{
					auto node = program->Nodes[i].get();
					for (auto instrNode = FirstInstructionNode(node->Code); instrNode != nullptr; instrNode = NextInstructionNode(instrNode))
					{
						auto & instr = GetInstruction(instrNode);
						if (instr.Func != ParallelCopy) continue;	
						func(instrNode, instr);
					}
				}
			}

			struct TransformContext
			{
				std::vector<std::shared_ptr<Variable>> SSA_Vars;	//是转成ssa之后的所有变量。
				std::vector<int> SSA_VarMap; // ssa var -> coalesced var  这里存的是数量更多的ssa var到”不干涉的都合成一个“的规约后变量映射。
				//以后重命名的时候，也直接把所有SSA_VarMap 里var-id的名字对应到直接的 var->Id 上就好了。 
				//是这样吗?

				std::vector<int> SSA_ValueNumber;
				std::vector<std::shared_ptr<EnumerableIntSet>> PhiClasses; // in ssa vars	
				InterferenceGraph InterGraph; // in coalecsed var

				TransformContext(){}
				TransformContext(TransformContext && ctx)
				{
					PhiClasses = std::move(ctx.PhiClasses);
					InterGraph = std::move(ctx.InterGraph);
					SSA_VarMap = std::move(ctx.SSA_VarMap);
					SSA_Vars = std::move(ctx.SSA_Vars);
					SSA_ValueNumber = std::move(ctx.SSA_ValueNumber);
				}
			};
			// phi的copy 应该是，执行phi形参拷贝到前驱块里的过程.
			//参数分别是:整个cfg的程序，复制之前的旧变量名字，还有关于什么的块?
			//功能: 一个复制了的新值，塞到vars里，并且添加定义一条语句来定义这个东西.

			//调用形式为 : MakePhiCopy(program, instr.Operands[i].Var, instrCopy);
			//传入的参数是指令的第i个操作数，Var 表示 指向变量的指针，后面的instrCopy是 类似“Instruction(ParallelCopy)”的东西.
			//总的来说，这个程序实现了“创建同样值但是是新的名字的变量，并且塞到cfg的var list里且加了一条define.
			//对phi 的参数执行了copy 和rename操作。
			Variable* MakePhiCopy(ControlFlowGraph * program, Variable * oldVar, InstructionNode * defNode)
			{
				auto targetVarOwner = std::make_shared<Variable>(*oldVar);	//赋值一个新的变量. 传入的是什么呢？ 
				auto targetVar = targetVarOwner.get();
				targetVar->Name = L"^phi_" + std::to_wstring(program->Variables.size()) + L"_" + targetVar->Name;	//Variable的名字是存在里面的，所以这里连name也复制过来了。
				// phi+程序里的变量个数？+ _ +此变量的名字 : ^phi_3_^tmp32_0[7]

				targetVar->Id = static_cast<int>(program->Variables.size());	//并且此变量的id设置为新加的最后一个变量。
				program->Variables.push_back(targetVarOwner);	//只是把此变量添加到cfg的变量集合里，并且id设置为最后一个，
				program->VarDefs.push_back(defNode);//defNode 是 类似“Instruction(ParallelCopy)”的东西，可是明明就是没有参数啊...
				return targetVar;
			}

			//此函数的意义是，为了消除phi而在前驱或者本块的开头等各种未知插入ParallelCopy指令和重命名操作数，替换掉phi里的参数 x->x' . 别的没有了。
			void InsertCopies(ControlFlowGraph * program)	
			{
				//遍历整个program ,对操作符为phi的指令，执行函数func. 
				//捕获，参数为指令节点和指令.. ? InstructionNode是List<inst>.
				ForEachPhi(program, [&](InstructionNode * instrNode, Instruction & instr)
				{
					for (size_t i = 0; i < instr.Operands.size(); i++)	//遍历当前phi指令的所有操作数.
					{
						auto node = instr.CFG_Node->Entries[i];	//node设置为此指令在 cfg 里的前驱
						InstructionNode* instrCopy = PreviousInstructionNode(LastInstructionNode(node->Code));//取倒数第二个指令.
						
						if (instrCopy == nullptr || GetInstruction(instrCopy).Func != ParallelCopy) //如为空,或者它的指令不是ParallelCopy，则插入一个ParallelCopy
						{
							instrCopy = InsertInstructionBefore(LastInstructionNode(node->Code), Instruction(ParallelCopy));
							//在身为phi前驱块的node的倒数第二的位置上插入一条ParallelCopy指令，并且返回这个插入的指令
						}

						auto targetVar = MakePhiCopy(program, instr.Operands[i].Var, instrCopy);
						//然后传入他们俩？ 意义何在？ 第二个参数是第i个操作数变量，第三个是插入了的 ParallelCopy 指令。
						//返回值是新加入的变量.
						
						//把phi的每个参数都重命名一下，并且塞到cfg的var 定义里，还在VarDefs 中加入一条 ParallelCopy 指令
						// {当此指令已经是 ParallelCopy 的时候，就不执行上面的if了。 那时候他会是啥呢？后面的程序遇到 ParallelCopy 指令的时候如何操作来使得ParallelCopy 的语义得以保持呢？ 存疑}
						
						//找到你了！！！ 原来就是在这里把 ParallelCopy 加上了操作数！ 那前面的没进 if的显然就是0+个Operands 的ParallelCopy指令了！！！这个循环会把phi的所有参数都塞到这个ParallelCopy指令里，待会被处理。
						GetInstruction(instrCopy).Operands.push_back(targetVar); 
						//然后这里又加上了原来名字的操作数。 这是待会2i,2i+1 操作的时候用的吗？？？ 去看看！果然，line 520. EvalValueNumber()里这么用了！
						GetInstruction(instrCopy).Operands.push_back(instr.Operands[i].Var);

						instr.Operands[i].Var = targetVar;	//在这里他俩除了name 之外是一样的，所以是在添加了ParallelCopy里的操作数之后，把phi的参数变成x-> x'了。 是这么理解吧。
					}//没有涉及计算phi-related web之类的..

					//上面的是针对Operands的，结果值只有一个，所以单独写一下。
					// for left operand:
					{
						//此函数的调用处 ForEachPhi 限定了 instr 一定就是phi指令，所以这句instr.CFG_Node->FirstInstruction() 是在找包含phi的此基本块的第一条指令。 如果它为空，就表示? 总之，如果为空就在此基本块的末尾加上一条ParallelCopy 指令。 
						//我觉得自己需要知道这个FirstInstruction()是什么东西，是不是只会是phi呢？未必。

						InstructionNode * instrCopy = instr.CFG_Node->FirstInstruction();	//摸到此指令所在的cfgNode的第一条指令.
						if (!instrCopy)		//如果为nullptr,则!为true,是空块？插入一条指令。
						{
							instrCopy = AppendInstruction(instr.CFG_Node->Code, Instruction(ParallelCopy));//在最后面加一条ParallelCopy指令.
						}
						else if (GetInstruction(instrCopy).Func != ParallelCopy)	
						{
							instrCopy = InsertInstructionBefore(instrCopy, Instruction(ParallelCopy));
						}

						//然后新创建一个变量，给phi的左操作数也重命名，并且把操作数都加到ParallelCopy  里面。
						auto newVar = MakePhiCopy(program, instr.LeftOperand.Var, instrCopy);
						GetInstruction(instrCopy).Operands.push_back(instr.LeftOperand.Var);
						GetInstruction(instrCopy).Operands.push_back(newVar);
						instr.LeftOperand.Var = newVar;	
					}
				});
			}

			//合并 phi .
			//这个函数说是 CoalescePhi ，实际上是在合并(coalesce) 相关的变量.
			//If the live ranges of variables in φ function (x3, x1, x2) do not interfere,then coalesce them into a single variable(X), and delete the φ - function.
			//这里是在coalesce them into a single variable(X)，但是为什么是在对phiClass进行操作?
			TransformContext CoalescePhi(ControlFlowGraph * program)
			{
				TransformContext context;	//储存了新旧变量名字，名字的映射关系，phi-web里提到的“等价类”  ，然后还有分析干涉图。

				for (auto & var : program->Variables)	//对程序的每一个变量
				{
					auto phiClass = std::make_shared<EnumerableIntSet>();	
					//选用此数据结构大概为了加减和union的集合， 
					//phiClass，DFA里提到了它，ssabook里对应的概念是phi-web. 
					//对程序里每个变量x都存了{x}的集合，后面只要确定了不干涉就合并起来

					phiClass->Set.SetMax(static_cast<int>(program->Variables.size()));
					//这里设置的是IntSet的最大值，考虑到IntSet是以位来保存n个数的，所以这里应该是为了方便集合操作，以及待会的集合操作可能会以IntSet来标记。

					phiClass->Add(var->Id);		// {x} 形式出现，对每个变量都这么搞一下. 注意，保存的是var的Id.

					//该关系的传递闭包（也就是phi related）定义了一个等价关系，该等价关系将函数中局部定义的变量划分为等价类phi - webs。
					//直观地，资源的phi等价类表示通过 phi 函数" 连接 "的一组资源。
					//所以这个phi class 保存了所有“通过phi联系起来”的一组变量,初始状态只有{x}本身。
					
					context.PhiClasses.push_back(phiClass);//所有的都保存进来。
				}
				
				//遍历程序中所有包含phi指令的节点，并且对phi指令施加此函数.
				//算法3.4：基于联合查找模式的φ-webs发现算法,
				ForEachPhi(program, [&](InstructionNode * instrNode, Instruction & instr)	
				{
					auto phiClass = context.PhiClasses[instr.LeftOperand.Var->Id];	// 取出对应被赋值的变量的phiclass.
					for (auto & op : instr.Operands)								//然后遍历此指令的所有操作数
					{
						phiClass->UnionWith(*context.PhiClasses[op.Var->Id]);	
						
						//把当前语境下的phiclass里的对应于op的phiClass求并集.
						//为什么要求并集呢? 这几个参数会不会互相干涉，要看cfg里的位置吧？ 直接在参数里也不能说明什么.. 不对，如果他们在这里，那就说明“必须要待会可以被规约”。 暂时挂起? 不对，根据算法
						//这里求并集，相当于合并PhiClasses. 是的。 合并的结果存在左边，下面重新塞回到classes里
						//注意看此函数被调用的位置，它应该是在规约阶段进行的，这个时候.. 所有的PhiClasses ,针对每个{x}的都已经建立，按照伪代码里的算法这个时候应该做的事是，判断参数们是否互相干涉，然后在不干涉的前提下合并彼此。 为什么这里直接就合并了?
						//算法里对这一块就是这么写的，所以人家没写错，是我理解的不到位...
						// 我不知道，我等死吧

						context.PhiClasses[op.Var->Id] = phiClass;
					}
				});

				context.SSA_Vars.clear();
				context.SSA_Vars.reserve(program->Variables.size());
				for (auto & variable : program->Variables)
					context.SSA_Vars.push_back(variable);//因为此刻程序的变量就是ssa形式的，所以直接赋值。 也验证了我"SSA_Vars里面存的是原初的变量"的想法.
				context.SSA_VarMap.resize(program->Variables.size());		//这个map大概是为了新x'跟x映射用的。
				return context;	
			}

			//一个辅助函数，对于每个要拷贝的对？ 这个pair是记录的一对变量，还是x'=x 这样的一对? 看起来是这个.
			//一个使用场景是处理opParallelCopy 的参数，一对一对的搞成赋值指令的形式.
			template <typename F>
			void ForEachCopyPair(Instruction & instr, F func)
			{
				for (size_t i = 0; i < instr.Operands.size() / 2; i++)//遍历此指令的所有参数
				{
					if (instr.Operands[i*2].Var != instr.Operands[i*2+1].Var)	//如果第2i个变量与第2i+1个不相等，则施加此函数。
						func(instr.Operands[i*2].Var, instr.Operands[i*2+1].Var);	//{ 0 1}，{2 3},... 
				}
			}

			//并行copy 线性化(lize). 这个对应的算法是? 3.6 ppt里也有对应的东西. 
			//通篇都没有涉及Interferes,可以放心观看~
			//跟这个一模一样！ https://github.com/pfalcon/parcopy 
			void SequentializeParallelCopy(ControlFlowGraph * program)
			{
				std::unordered_map<DataType, Variable *> temp_vars;
				//这个里面存的是这种类型的值？ 可是DataType 不就一种吗? 哦，有三种，可是那又怎么样？ 您是打算在TryGetValue里存一堆type然后在仅有三种的type里找？ 不懂..
				std::vector<Variable *> ready, todo;
				std::unordered_map<Variable *, Variable *> loc;
				std::unordered_map<Variable *, Variable *> pred;
				ForEachParallelCopy(program, [&](InstructionNode * instrNode, Instruction & instr)
				{
					//起手清空
					ready.clear();
					todo.clear();
					loc.clear();
					pred.clear();

					//遍历传入的参数是,捕获到的本条 func 为 ParallelCopy 的指令。
					//ParallelCopy 是有参数的，这里是在一对一对处理相邻的变量。 这里是func的函数体. 对两个变量的操作。第一遍，把每个b的位置和a的前面一个(?)都设置为空.
					// https://github.com/pfalcon/parcopy 与论文里的东西没区别.

					//这个对应第一个循环，
					//resource(b)<-nil，resource 就是loc 所以这里对loc的归零就是对resource的nil.

					ForEachCopyPair(instr, [&](Variable * b, Variable * a)
					{
						loc[0] = 0;
						pred[0] = 0;
						// resource(b) ← nil 表示每个目标都没有初始值可查，必然被覆盖；
						loc[b] = 0;
						pred[a] = 0;
					});

					ForEachCopyPair(instr, [&](Variable * b, Variable * a) // in the form of b = a
					{
						// resource(a) ← a 表示每个来源值的初始值就是它自己；
						loc[a] = a;
						//走遍这个图的过程中，resource(a) <- b ，表示 a 这个值现在可以在 b 处取得
						pred[b] = a;
						todo.push_back(a);
					});
					//前两个 for each 在建立这一连串拷贝发生前的事实,第三个是记录叶子节点
					//第三个循环，ready 在这里是available	
					ForEachCopyPair(instr, [&](Variable * b, Variable * a) // in the form of b = a
					{
						
						if (loc[b] == 0)
							ready.push_back(b);
					});
					auto EmitCopy = [&](Variable * dest, Variable * src)	
					{
						InsertInstructionAfter(instrNode, Instruction(dest, 0, src));
					};

					while (!todo.empty())
					{
						Variable * a, * b, * c;
						while (!ready.empty())
						{
							b = ready.back();
							ready.pop_back();
							a = pred[b];
							c = loc[a];
							EmitCopy(b, c);
							loc[a] = b;
							if (a== c && pred[a] != 0)	
								ready.push_back(a);
						}
						b = todo.back();
						todo.pop_back();

						if (b == loc[pred[b]])
						{
							//==的这一部分是代码作者自己加的，原文没有这个。
							Variable * tmpVar;
							if (auto iter = temp_vars.find(b->Type); iter != temp_vars.end())
							{
								tmpVar = iter->second;
							}
							else
							{
								tmpVar = MakeParallelCopyTemp(b);
								temp_vars[b->Type] = tmpVar;
							}
							EmitCopy(tmpVar, b);
							loc[b] = tmpVar;
							ready.push_back(b);
						}
					}
					RemoveInstruction(instrNode);
				});
			}

			//这个有对应的恰到好处的论文吗？ 还是需要自己读的？ 看起来没有，先快速过一遍，到 Live-range intersection 那一块再同步吧。
			void Rename(ControlFlowGraph * program, TransformContext & context)
			{
				std::vector<std::shared_ptr<Variable>> newVars;		//重命名之后，得到的non-ssa的每个变量的值？
				std::unordered_map<Variable*, Variable*> var_mapping;
				std::unordered_map<Variable*, Variable*> old_var_mapping;
				std::vector<std::shared_ptr<EnumerableIntSet>> newPhiClasses;

				for (int i = 0; i < static_cast<int>(program->Variables.size()); i++)	//遍历所有变量
				{
					auto curVar = program->Variables[i].get();
					if (i < program->ParameterCount)	//形参数量？ 那么VariableSize代表的是local 变量多少吗？ 加在一起是Variables.size() ?
					{
						newVars.push_back(program->Variables[i]);		//本变量被塞进newVars 里
						var_mapping[program->Variables[i].get()] = program->Variables[i].get();	
						old_var_mapping[program->Variables[i].get()] = program->Variables[i].get();
						//并且varMapping里存了每个变量映射到prog里的变量指针的位置.
						//所以这两个 VarMapping 其实存的是程序里的变量的指针到它们 自己本身的映射？ 初始化的时候看起来就是如此。
						//这仅仅是初始化.
					}

					else //>= ParameterCount  对应非形参的变量们， 前面就直接塞入映射，是因为... 形参是跨函数的，不参与phi-webs之类的操作，就这么认为吧.
					{
						auto phiClass = context.PhiClasses[i];	//获得第i个变量的phiClass的指针. 在初始化完成的时候，这个里面存的是每个变量自身{x}.
						//可是我对形参们也计算了这个啊... 反正不操作，就当它没有，挺好。

						if (phiClass)	//如果不为空，则
						{
							int representId = *std::min_element(phiClass->Values.begin(), phiClass->Values.end());
							//min求的是list里的最小值。  为什么要找最小呢. 因为 phiClass 里存的变量索引是以递增顺序进行的? 先不管.
							//作为代表整个phi-webs的存在.

							Variable * representVar = context.SSA_Vars[representId].get();		//获取它在ssa_vars里存放的变量指针的值。
							Variable * newVar;	//新的会被怎么用呢?

							if (auto iter = var_mapping.find(representVar); iter != var_mapping.end())
							{
								newVar = iter->second;
							}
							else
								//是在mapping里寻找此representVar，找到了的话就把值丢到newVar里，找不到就false.
								//此mapping里，pair是<变量自己，变量自己> 对吧？ 那还他妈个逼的找个p啊？ 显然，如果找到了左边，就把值放入右边，这不是挺好吗..
								//以及，varMapping 是<Variable*,Variable*>，但是它映射的是什么东西？ 总之，如果找不到，就手动搞个新的

							{
								auto newVarOwner = std::make_shared<Variable>(*representVar);	//如果找不到就搞个新的
								newVar = newVarOwner.get();
								if (representVar->Version > 0)		
									//这个Version是ssa变量的版本，应该是x[0] 后面的0,生成中的例子：
									//^ phi_1_^ tmp32_0[5]
									//^ phi_1_^ tmp32_0[5]
									//^ phi_2_^ tmp32_0[6]
									//^ phi_3_^ tmp32_0[7]
									//^ phi_1_^ tmp32_0[5]
									//^ phi_5_^ tmp32_0[8]
									//但是我还是不知道这个version是干嘛的？ 每次对同一个变量可能有多次赋值，于是有了不同的版本？ 
									//看的书上，都是最多x1=x1'一下啊，也没见过x''的。 
									// llvm 没看到相关内容，在线llvm ass 只有phi。
									// 已确认 https://blog.csdn.net/qq_29674357/article/details/78731713

									newVar->Name = L"^phi_" + std::to_wstring(representVar->Version) + L"_" + newVar->Name; 
								newVar->Version = 0;	//搞完把没用了的版本号改成0,以后估计也用不上了.
								newVar->Id = static_cast<int>(newVars.size());
								newVars.push_back(newVarOwner);	//并且加到新名字里。
							}
							old_var_mapping[newVar] = curVar;//新旧名字互相映射. 待会会用到对吧? 
							var_mapping[curVar] = newVar;
							var_mapping[representVar] = newVar;	
							//representVar 是ssa_vars里的var-id  把它也设置成newVar.
						}
						//这里只实现了“把带version的名字都改成独一无二的.既没有替换掉phi - webs.也没有执行什么消除操作，就只是往varMapping里填了东西而已。
						else		//这个else对应的是phiClass为空的情况,这种时候.. 即第i个变量没有自己的PhiClass,为什么，不知道.
						{
							auto newVarOwner = std::make_shared<Variable>(*curVar);
							auto newVar = newVarOwner.get();
							newVar->Version = 0;
							newVar->Id = static_cast<int>(newVars.size());
							newVars.push_back(newVarOwner);
							var_mapping[curVar] = newVar;
							old_var_mapping[newVar] = curVar;
						}
					}
				}
				
				//上面完成了变量的映射，这里一一删掉Phi指令
				for (auto & node : program->Nodes)	//接下来是对所有节点里所有的指令进行。
				{
					for (auto instrNode = FirstInstructionNode(node->Code); instrNode != nullptr; )
					{
						auto nextInstrNode = NextInstructionNode(instrNode);
						auto & instr = GetInstruction(instrNode);
						if (instr.Func == Operation::Phi)
						{
							//所有的phi相关的变量都被在vars里弄好映射关系，也靠着伪并行指令完成复制操作的插入了，自然可以直接删掉phi指令了。
							RemoveInstruction(instrNode);	//删掉的是phi所在的？Instruction，是的，仅仅删掉一条指令而已。
							instrNode = nextInstrNode;
							continue;
						}
						//如果不是phi 而是其他指令的话，
						if (instr.LeftOperand.IsVariable())
						{

							//不，这里是普通其他指令的变量名字改写操作.
							instr.LeftOperand.Var = var_mapping[instr.LeftOperand.Var];	
							//如果左操作数是变量就从varMapping里找出来，varMapping里保存的是当前值与重命名之后的值的映射.
						}
						for (auto & op : instr.Operands)//跟上面一样，区别是上面是被赋值的那个，这里是右边的一堆操作数.
							if (op.IsVariable())
								op.Var = var_mapping[op.Var];	//把每个操作数也都改成新的值.

						//如果指令是空，且左边和右边的第一个操作数相同且都是变量，就直接删掉当前语句。处理自赋值的情况.
						if (instr.Func == 0 && instr.Operands[0].IsVariable() && instr.LeftOperand.Var == instr.Operands[0].Var)
						{
							RemoveInstruction(instrNode);	
							instrNode = nextInstrNode;
							continue;
						}
						instrNode = nextInstrNode;
					}
				}
				//自此，phi被删除，每一个变量都有了mapping里的新名字，然后呢?

				//那么这个PhiClasses的[]操作里都有啥? 
				//PhiClasses[oldVar->Id] 获得的是指向某一个具体EnumerableIntSet的指针，而EnumerableIntSet 保存了关于作用域的信息了吗?
				//看起来是没有的，LiveRange和EnumerableIntSet互不干涉，那... 暂时挂起！

				for (auto var : newVars)
				{
					auto oldVar = old_var_mapping.at(var.get());	//先从map里提取出每一个旧的变量的值.

					auto oldLiveRange = context.PhiClasses[oldVar->Id];	
					//这俩有啥关系？ 为什么能从PhiClasses 里摸出来live range ？
					//考虑到我们的PhiClass是一个IntSet,也就是类似集合的东西，所以它储存东西的方式是，某几个位亮着就说明某个值在里面。
					//IntSet类似于，https://oi-wiki.org/math/bit/#_12 从低位到高位 012345678，如果对应的位存在数字就设置为1,否则设置为0，这样对集合间进行运算的时候执行位运算就行了。
					// 这里的[]里面是数字，代表取第i个，我们知道var的id跟phiClass的id是一一对应的。 所以取的就是对应old var 的phi class. 
					
					//这个live range 就是该 class 里存了哪几个变量?  
					//现在来回答这个问题，为什么直接从phi-webs里就可以摸出来live range!
					// 反正至今为止我都没看到过操作phi-class的干涉分析的变量相关... 或许。 这个live range就是在表达“我以多少形态存在于多少世界”？ 或许吧。 那么我们知道了，在未命名以前，他叫x1 之后，他叫x2. 下一个phi的结果里，他叫x3... 这样，它在整个程序长河里，就叫live range了。 这么理解或许也没错，这个live range 跟干涉本不相关。 显然，两者的关系是这样的！！！
					/*The live range of a variable is its def-use chain. It includes all the program points between the definition and each of its uses.
					Two live ranges interfere if there is a program point that is common to both the live ranges.
						Because of the single definition property of SSA form programs, the definition associates a live range with a variable.In contrast, for non - SSA form programs, the live range is defined as the maximal union of intersecting def - use chains.Since def - use chains for SSA form programs do not intersect, the simple definition given above suffices*/
					//好！ 我竟然自己脑补出来了一半的定义！！！
					//接下来的一切的一切都极其简单了！ 
					//暂时挂起， DFA 222.

					newPhiClasses.push_back(oldLiveRange);	
					// 加到新的phi class 里,这里Add的时候操作的是phi-class的哪个成员? 是List.
					//可是 oldLiveRange 是EnumerableIntSet 啊，哦，这里就是 EnumerableIntSet 的list..  那么，这个newPhi 就是在把old里的东西一一捡出来塞到新list里吗？ 接下来做了什么
					//我想说的是 LiveRange ，这个东西确实有 int Id;IntSet VarSet; 两个成员，但是在这里关系不大.

					for (auto & ssaVar : oldLiveRange->Values)		//接着遍历旧的range里的值,这里访问的是 List<int>  里的东西。
						//然后映射过去 怎么到了这里又开始用它的list<int>的部分了，嗯..它明明是.. var-Id? 再仔细看一眼，之前往phi-class里加东西的时候，是怎么操作两个子部件的？
						//注意，这里的 oldLiveRange 实际上是一个 PhiClass ，所以它里面的list<int> 就是 Add(var->Id) 进来的变量id.

						context.SSA_VarMap[ssaVar] = var->Id;	//首先明确一点，这里存的是Id了。，这里在做的事是，把PhiClass里存放的每一个 var都找到在SSA_VarMap 里的映射，这个SSA_VarMap存放的是什么来着? 是多个ssa 映射到同一个non-ssa var的关系，相当于规定 1 2 3 4 其中 2 4 都是 1，这一句写完之后，就变成了 1 1 3 1. 把每个存在于oldLiveRange里的值都设定成了1的index.
					
						//但是修改后的效果体现在哪里呢？ 这个context.SSA_VarMap 如何影响外界？context是传进来的引用，对他的修改直接生效，好的。

						// ssa var -> coalecsed var  这里存的是数量更多的ssa var到”不干涉的都合成i一个“的规约后的var. 所以list里存的应该是， oldLiveRange->Values 里所有的 ssaVar 都对应到同一个var-Id上。 
						//以后重命名的时候，也直接把所有SSA_VarMap 里var-id的名字对应到直接的 var->Id 上就好了。 
						//大概就是这样，这就是所谓的”rename".
				}
				
				//然后把这个新的接到指针上.
				program->Variables = std::move(newVars);
				context.PhiClasses = std::move(newPhiClasses);
			}

			//这个而应该是干涉性分析.
			//传入参数，一个上下文，两个phiClass. 
			//总之这个函数就是分析两个phi-Web是否存在干涉的，估计会以两个变量的，类似phiClass[i,j]的形式传进来，然后分析它们是否存在干涉现象，然后决定是否插入复制操作，大概是这样吧...
			bool PhiClassInterferes(TransformContext & context, const std::shared_ptr<EnumerableIntSet> & phi1, const std::shared_ptr<EnumerableIntSet> & phi2)
			{	
				//既然已经清楚phi是集合了，那么下面的操作就是看集合里是否有相同的变量。 如果有的话就代表? 应该不代表任何东西吧... 正经的计算live range不是应该计算前驱后继和赋值状况吗?  我又不懂了，phi里记录的是关系网，而同一个关系网里的东西一定都是有相同的生命周期的?
				//同时存在于多个关系网里的就一定是发生了phi之外的赋值？ 这么理解对吧?

				//看了别人的博客我更晕了，怎么他们又说phi-web代表的是同一个变量，在cssa里同一时刻只有一个活跃，而冲突是由于变换产生的。
				
				//这就对我“如果一个var出现在了两个web里的话就说明干涉了”不一样啊.. 甚至概念都是错的..
				
				//我不知道它是怎么定义和判断干涉的了，啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊啊。

				for (int i = 0; i < static_cast<int>(phi1->Values.size()); i++)	//遍历1里面所有的变量.
				{
					int var1 = context.SSA_VarMap[phi1->Values[i]];				//从ssa_var里找对应的值设为var1.
					for (int j = 0; j < static_cast<int>(phi2->Values.size()); j++)					//然后在phi2里遍历每个var2.
					{
						int var2 = context.SSA_VarMap[phi2->Values[j]];						//我知道，这个里面存的是？
						if (var1 != var2 && context.InterGraph.Interferes(var1, var2)			//如果两个不相等，并且
							&& context.SSA_ValueNumber[var1] != context.SSA_ValueNumber[var2])	//而且此context下var1和var2的值不相等，则存在干涉.
							
							//SSA_VarMap; // ssa var -> coalecsed var 原文有说是coalecsed var,合并之后的变量.   所以里面存的是ssa里的到闭合之后的映射。
							return true;
					}
				}
				return false;
			}
			
			//尝试合并变量。
			//这里合并的是phi-class吗? 不说这个，先问一下，如何合并？ 只要是cssa,直接替换就行了，嗯。
			void TryCoalesceVariable(TransformContext & context, Variable * var1, Variable * var2)
			{
				auto & phi1 = context.PhiClasses[var1->Id];
				auto & phi2 = context.PhiClasses[var2->Id];
				if (phi1 != phi2 && !PhiClassInterferes(context, phi1, phi2))
					//如果俩的干涉为false.且互不相等，就可以union,我能明白互不干涉的重要性，可是为什么限定不相等？ 因为这个函数被调用的地方实际上都是在赋值？？？ 也许吧，如果相等的话就没必要合并phi-class了。 确实如此！！！ 因为是在赋值，所以就代表同一个变量的，自然要塞到一起，晚点替换成同一个。
					//什么时候会执行这个替换呢?
					//这个函数的调用是在倒数第二个函数调用之后，下一次rename之前。
					//我还记得delete phi是在什么时候? 是在第一个rename里。
					//那么phi-class被重命名成单个名字会是在什么时候？ 在最后一个rename()调用的时候呗。
					//具体是如何做的呢? 去看看那个函数！
				{
					phi1->UnionWith(*phi2);//把phi2里的东西都塞到phi1里。 phi2大小还是原来那样.
					for (auto var : phi1->Values)	//这个是啥? 把phi里的所有变量都塞到另一个的PhiClass里。
					{
						context.PhiClasses[context.SSA_VarMap[var]] = phi1;
					}
					for (auto var : phi2->Values)	//这个则是把ph2的每个变量的phiClass都设置成ph1.
					{
						context.PhiClasses[context.SSA_VarMap[var]] = phi1;
					}
					//为什么要做这种事，为了.. 让此变量的phiClass变大吧，毕竟上面已经union了。 嗯，phi1是结果，所以这两个for都是赋值phi1.
				}
			}
			
			//按照ssa book所说，是在dtor ssa之后进行，跟ParallelCopy 一样的，可以放在任何时候执行的。
			//这个应该只是简单的消除一些冗余操作，没啥值得一说的。
			void CoalesceCopy(ControlFlowGraph * program, TransformContext & context)
			{
				for (auto & node : program->Nodes)
				{
					for (auto & instr : node->Code)	//每个指令
					{
						if (instr.Func == nullptr)
						{
							if (instr.Operands[0].IsVariable())	//如果指令为空且是变量，就显然是 a=b 的形式了，尝试合并变量看看！
								TryCoalesceVariable(context, instr.LeftOperand.Var, instr.Operands[0].Var);
						}
						else if (instr.Func == ParallelCopy)	//如果指令是 ParallelCopy 就合并操作数试试。
						{
							for (size_t i = 0; i < instr.Operands.size() / 2; i++)
							{
								TryCoalesceVariable(context, instr.Operands[i*2].Var, instr.Operands[i*2+1].Var);
							}
						}
					}
				}
			}
			
			void EvalValueNumber(ControlFlowGraph * program, TransformContext & context)
			{
				context.SSA_ValueNumber.resize(context.SSA_Vars.size());
				for (int i = 0; i < static_cast<int>(context.SSA_ValueNumber.size()); i++)
					context.SSA_ValueNumber[i] = i;
				auto traverse = program->GetPostOrder();	//又是一个后续遍历
				for (int i = static_cast<int>(traverse.size()) - 1; i>=0; i--)	
				{
					auto & node = traverse[i];	//然后访问这个遍历结果.
					for (auto & instr : node->Code)	//依旧是遍历cfg中每个结点的每一条指令。
					{
						if (instr.Func == 0 && instr.Operands[0].IsVariable())	//如果遇到了 a=b 形式的指令
							context.SSA_ValueNumber[instr.LeftOperand.Var->Id] = context.SSA_ValueNumber[instr.Operands[0].Var->Id];
						// 就把 SSA_ValueNumber 里面的对应关系写出来.

						else if (instr.Func == ParallelCopy)
						{
							//instr 在里面，会被怎么对待? 摸到所有操作数，如下处理。
							ForEachCopyPair(instr, [&](Variable *a, Variable * b)
							{
								context.SSA_ValueNumber[a->Id] = context.SSA_ValueNumber[b->Id];
							});
						}
					}
				}

			}
			
			//最终的敌人！
			void OutOfSSA(ControlFlowGraph * program)
			{
				InsertCopies(program);	//插入拷贝操作(在前驱块里?) 这里应该是插入了一大堆。

				TransformContext context = CoalescePhi(program);	//说是CoalescePhi其实合并的是phiClass.
				EvalValueNumber(program, context);		//我只看到它做了两件事，进行左右操作数的计算，在SSA_ValueNumber里。 这个是干嘛的至今不知道...
				Rename(program, context);			//进行类似 x=x'的变量重命名，还有几个mapping 里的互相映射和修改.
				SequentializeParallelCopy(program);		//把并行拷贝变成序列化。 里面有todo,有ready,有insert copy stmtment. 不确定在干嘛..

				//接下来就是图的部分.
				context.InterGraph = InterferenceAnalysis::BuildInterferenceGraph(program);		//phi-web 的计算，这个是灵魂了！就差你了！

				//x86里也有这个，所以它大概是为了给那个用的。	总之，这里做了什么?
				/*for (auto node : program->Nodes)
				{
					printf("live out(%d): ", node->Id);
					for (int i = 0; i < static_cast<int>(program->Variables.size()); i++)		//原来你们是这么调试东西的，看id啊..我也这么写！
					{
						if (node->LiveOut.Contains(i))
							printf("%d ", i);
					}
					printf("\n");
				}*/
				CoalesceCopy(program, context);		//合并多余的拷贝(很简单)
				Rename(program, context);	//这个前面也看过了，所以只看那两个函数就好.
				//最后为什么还要rename一次?
			}
		public:
			virtual ProgramOptimizationResult Optimize(std::shared_ptr<ControlFlowGraph> program) override
			{
				ProgramOptimizationResult rs;
				rs.Changed = true;
				rs.Program = program;
				OutOfSSA(program.get());
				return rs;
			}
		};

		std::unique_ptr<IntraProcOptimizer> CreateOutOfSSA_Transform()
		{
			return std::make_unique<OutOfSSA_Transform>();
		}
	}
}
