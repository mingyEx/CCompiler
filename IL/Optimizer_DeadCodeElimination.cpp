#include "Optimization.h"
#include "Exception.h"
#include "TransformInvariant.h"

namespace Compiler
{
	namespace Intermediate
	{
		//第三柱！ 就是书上 10.2.1的内容了 几乎跟原文一一对应，改天脑子不好使了再看~
		class DeadCodeOptimizer : public IntraProcOptimizer
		{
		private:
			//定义了“关键边”. 为什么是这三个指令？p402有原文.
			bool IsCritical(Instruction & instr)
			{
				return (instr.Func == Operation::Ret ||
						instr.Func == Operation::Call ||
						instr.Func == Operation::Store);
			}

			//寻找最近的标记了的反向支配边界,赋值并且返回.
			ControlFlowNode * FindNeareastMarkedPostDom(ControlFlowNode * node, IntSet & nodeMarks)
			{
				auto curNode = node->ReverseImmediateDominator;
				while (!nodeMarks.Contains(curNode->Id))
				{
					curNode = curNode->ReverseImmediateDominator;
				}
				return curNode;
			}
			
			//删掉随着时代变迁已经空了的节点.
			void CompactNodes(ControlFlowGraph * graph)
			{
				List<ControlFlowNode*> traverse = graph->GetPostOrder();	//获得图的后序排列.
				IntSet nodeMarks;
				nodeMarks.SetMax(graph->Nodes.Count());
				int count = 0;
				for (auto & node:traverse)
				{
					count ++;
					nodeMarks.Add(node->Id);	//里面是图的遍历路径里包含的节点
				}
				for (auto & node:traverse)
				{
					if (nodeMarks.Contains(node->Id))
					{
						List<ControlFlowNode*> newEntries;
						for (int i = 0; i<node->Entries.Count(); i++)	//计算每个节点的所有进入节点.
							if (nodeMarks.Contains(node->Entries[i]->Id))	//仅限于此节点在图的遍历中时候猜添加.	不在图中的包括哪些，为什么会存在“不在图中却在进入节点里”的情况？ 因为是从根节点开始后序遍历的..? 不管了,我怎么知道作者在想什么...
								//哦，因为这是一个已经消除过无用节点的图，有一些节点已经失去进入其他节点的机会了，但是依旧存在于图中. 或许吧。
								//我没看到那些空掉的节点是否被从node->Entries里移除了.. 这个循环的全部意义仅仅如此。
								newEntries.Add(node->Entries[i]);
						
						node->Entries = _Move(newEntries);	//然后替换掉那些.
						for (int i = 0; i<2; i++)
						{
							if (node->Exits[0])
							{
								if (!nodeMarks.Contains(node->Exits[0]->Id))	//如果exit 0 不在的话就设置为1，因为至少会有一个退出节点.
									node->Exits[0] = node->Exits[1];
							}
							else
								break;
						}
						if (node->GetExitCount()==1)
							node->Code.Last().Func = Operation::Jump;	//只有一个结点的话就干脆设置成jump.
					}
				}
				List<RefPtr<ControlFlowNode>> newNodes;
				newNodes.Reserve(count);
				for (int i = 0; i<graph->Nodes.Count(); i++)
				{
					if (nodeMarks.Contains(i))
					{
						graph->Nodes[i]->Id = newNodes.Count();
						newNodes.Add(graph->Nodes[i]);
					}
				}
				graph->Nodes = _Move(newNodes);

			}

			void Clean(ControlFlowGraph * graph)
			{
				CompactNodes(graph);	//清理节点.然后主算法就开始了.
				bool changed = true;
				while (changed)
				{
					changed = false;
					List<ControlFlowNode*> traverse = graph->GetPostOrder();
					for (auto & node : traverse)
					{
						//if (node == 0)	//应该是空指针.
						if (node == nullptr)
							continue;
						if (node == graph->Source || node == graph->Sink)
							continue;
						// Step 1: Fold redundant branch
						if (node->GetExitCount() == 2 && node->Exits[0] == node->Exits[1])
						{
							node->Exits[1] = 0;
							if (node->Code.Count())	//这个的意思应该是“指令数量不为零"
							{
								node->Code.Last().Func = Operation::Jump;	//末尾加一条jump.
							}
							node->Exits[0]->Entries.Remove(node);	//从它退出的节点的进入里移除当前节点。 为什么要写两遍?
							node->Exits[0]->Entries.Remove(node);	//因为这两个指向的都是同一个节点，而那个节点里有两个进入点都是当前，需要全都删掉.
							node->Exits[0]->Entries.Add(node);		//然后把本节点加进去？ node是一个引用，直接修改就行了为什么要删两遍? 大概是为了防止”只有一个节点为enter的情况，暂时不管.
							changed = true;
						}
						if (node->GetExitCount() == 1)
						{
							// Step 2: Remove empty block
							if (node->Code.Count()==0 || node->Code.Count()==1 && node->Code.First().Func == Operation::Jump)
							{
								//对于空节点，活着有一条指令且为jump的.

								int entryId = node->Exits[0]->Entries.IndexOf(node);	//设置进入节点为本节点的下一个节点的开始里，对应的槽位.
								node->Exits[0]->Entries.RemoveAt(entryId);		//这个是?移除entryId节点.
								node->Exits[0]->Entries.InsertRange(entryId, node->Entries);	//把进入此节点的都转到下一个里去.
								for (auto & upperNode : node->Entries)	//这个是? 此节点的进入，看看它的退出节点是否存在，全部设置为此节点的退出.
									//前面其他操作执行完这个了吗？ 上面的关注点都是“此节点与下一个结点的关系”。而这里处理的是“此节点与前面结点的关系”。
								{
									for (int j = 0; j<2; j++)
										if (upperNode->Exits[j] == node)
											upperNode->Exits[j] = node->Exits[0];
								}
								graph->Nodes[node->Id] = 0;	//处理完之后一切都归零.
								node = 0;
								changed = true;
								continue;
							}
							// Step 3: merge block
							//这里是什么.. 合并基本块？ 前面不是已经都.. 哦，前面只是在处理节点间的那些触手. 这里是在说啥?
							if (node->Exits[0] != graph->Sink && node->Exits[0]->Entries.Count() == 1)
							{
								//如果当前节点的退出不是图的退出，并且退出节点的数量为1，{则说明是单线关系，但是这里做的事也是在合并Node吧？前面是在删掉某个Node,这里是在合并两个里面有指令的Node.}
								auto exitNode = node->Exits[0];
								node->Code.LastNode()->Delete();	//最后一条指令显然是跳转，删掉他然后把吓一跳的塞进来就好.
								for (auto & instr : exitNode->Code)
									node->Code.AddLast(instr);
								node->Exits[0] = exitNode->Exits[0];	//退出节点重新设置为下一个的.
								node->Exits[1] = exitNode->Exits[1];
								for (auto child : node->Exits)	//然后处理退出节点?
									if (child)
									{
										int entryId = child->Entries.IndexOf(exitNode);	//退出节点如果不为空，就取得其在下一个节点的Id,然后把本节点设置为插槽里的对应位置。 仅此而已. 从本节点里跑到下一个的链接的插槽位置都换成本节点，为什么要执行这个操作? 因为上面的赋值已经改变了这些，所以需要从新连起下一节点到本届点的指针.
										child->Entries[entryId] = node;
									}
								graph->Nodes[exitNode->Id] = 0;	//然后把这个节点在图里的位置置空，待会就会被处理掉.
								int idx = traverse.IndexOf(exitNode);	//并且在traverse里的也标记为0.
								traverse[idx] = 0;
								changed = true;
								continue;
							}
							// Step 4: hoist branch 提升分支。
							//艹，简直严格按照书上来的，一一对应.
							//此优化的意义是，如果下一个块为空且只有两个分支，就把分支搞到此节点上.
							if (node->Exits[0]->Code.Count() == 1 && node->Exits[0]->GetExitCount() == 2)
							{
								auto exitNode = node->Exits[0];
								node->Code.Last() = exitNode->Code.Last();	//把下一块的所有指令都塞给本节点的最后一条{因为两个Node的最后一一条指令都是跳转}.

								for (int k = 0; k<2; k++)	//然后执行触手重置.
								{
									node->Exits[k] = exitNode->Exits[k];
									int entryId = exitNode->Exits[k]->Entries.IndexOf(exitNode);
									exitNode->Exits[k]->Entries[entryId] = node;
								}
								changed = true;
							}
						}
					}
				}
				CompactNodes(graph);	//上面的操作会产生一些空节点，再次清理一遍.
				
				for (auto & node : graph->Nodes)
				{
					TransformInvariant::PlacePhiAtTop(node.Ptr());	//这个是啥？ 看起来是..对每个节点进行了.. ssa的操作.
					for (auto & instr : node->Code)	//对节点里的每一个指令，把节点的指针都赋值给了指令对应的CFG_NOde,大概是上面的操作改变了指令里的Node值吧...
						instr.CFG_Node = node.Ptr();
				}
				graph->ComputeDominatorTree();	//然后重新计算一遍支配者树.里面会重新设定节点的各个指针.
			}

			//删掉某个节点里，指定的id对应的Enter.
			void RemoveEntry(ControlFlowNode * node, int id)
			{
				node->Entries.RemoveAt(id);
				for (auto & instr : node->Code)	//并且如果此节点里有phi的话，就删掉它参数里的enter的id.
					if (instr.Func == Operation::Phi)
						instr.Operands.RemoveAt(id);
			}
		public:
			DeadCodeOptimizer() {};
			
			//然后是该算法的主要部分.
			virtual ProgramOptimizationResult Optimize(RefPtr<ControlFlowGraph> program) override
			{
				ProgramOptimizationResult result;
				result.Program = program;
				result.Changed = false;
				List<Instruction *> workList;	//伪代码里的worklist。
				IntSet nodeMark;				//标记
				nodeMark.SetMax(program->Nodes.Count());
				auto traverse = program->GetPostOrder();
				for (int i = traverse.Count()-1; i>=0; i--)
				{
					int instrId = 0;
					for (auto & instr : traverse[i]->Code)
					{
						if (IsCritical(instr))
						{
							nodeMark.Add(instr.CFG_Node->Id);	//只标记是关键操作的指令。 这里是在标记Node.
							instr.Mark = 1;
							workList.Add(&instr);
						}
						else
							instr.Mark = 0;
						instrId++;
					}
				}
				while (workList.Count())
				{
					auto &instr = *(workList.Last());
					workList.RemoveAt(workList.Count()-1);
					for (auto & op : instr.Operands)
					{
						if (op.IsVariable() && program->VarDefs[op.Var->Id] != 0)
						{
							//这个是干嘛的? 是图里的变量定义的指令list. {这里也是，for循环本身只对x <- y op z这种“定义”的指令进行处理，所以if里有“...!=0”，而下面只是为了获取它在cfg里的值.}
							auto defInstr = &program->VarDefs[op.Var->Id]->Value;
							//是p403里的两个if def.

							if (defInstr->Mark == 0)
							{
								nodeMark.Add(defInstr->CFG_Node->Id);
								defInstr->Mark = 1;
								workList.Add(defInstr);
							}
						}
					}
					//完全对应下面的代码. 本条指令所对应的CFG_Node的Node，的list<rdf>.
					for (auto node : instr.CFG_Node->ReverseDominateFrontier) //node 就是 b
					{
						/*if (node->Code.Count() == 0 ||
							node->Code.Last().Func != Operation::JumpF &&
							node->Code.Last().Func != Operation::JumpT)
							throw InvalidProgramException(L"Inconsistant CFG");*/
						auto &bInstr = node->Code.Last();	//j is the branch that ends b. j== bInstr
						if (bInstr.Mark == 0)
						{
							bInstr.Mark = 1;
							nodeMark.Add(bInstr.CFG_Node->Id);	//书上一条，这里是俩,为什么呢.
							workList.Add(&bInstr);
						}
					}
				}
				//这里对应Sweep()
				for (int i = 0; i<program->Nodes.Count(); i++)
				{
					int instrId = 0;
					auto & code = program->Nodes[i]->Code;
					for (auto & iter = code.begin(); iter != code.end(); iter++)
					{
						auto & instr = iter.Current->Value;
						if (instr.Mark == 0)
						{
							if (instr.Func == Operation::Branch)
							{
								result.Changed = true;
								instr.Func = Operation::Jump; //重写成jump
								// find nearest marked post dominator
								//
								auto postDom = FindNeareastMarkedPostDom(instr.CFG_Node, nodeMark);
								for (int j = 0; j<2; j++)
									if (instr.CFG_Node->Exits[j] != 0)
										instr.CFG_Node->Exits[j]->Entries.Remove(instr.CFG_Node);
								//把分支替换成jump. 跳转到该程序块的第一个包含有用指令的后向支配着。

								instr.CFG_Node->Exits[0] = postDom;
								instr.CFG_Node->Exits[1] = 0;
								postDom->Entries.Add(instr.CFG_Node);
							}
							//如果分支指令未被标记，则从其后继节点直到其直接的后向支配节点，都不包含有用操作，所以直接删除.
							else if (instr.Func != Operation::Jump)
							{
								result.Changed = true;
								iter.Current->Delete();
							}
						}
					}
				}
				/*if (result.Changed)
					Clean(result.Program.Ptr());*/
				
				return result;
			}
		};
		
		IntraProcOptimizer * CreateDeadCodeOptimizer()
		{
			return new DeadCodeOptimizer();
		}
	}
}