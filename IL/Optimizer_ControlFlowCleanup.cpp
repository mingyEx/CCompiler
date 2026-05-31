#include "Optimization.h"
#include "BitIntSet.h"
#include "TransformInvariant.h"
#include <algorithm>
#include <unordered_set>
#include <vector>

namespace Compiler
{
	namespace Intermediate
	{
		//看了一圈，无用代码清理是ssa之后的事了，所以这个是干嘛的，自己看吧,确实，那几个都有自己的优化器. 太明显了，跟书上几乎一样！
		//此class 确实是在ssa之后的事啊！CreateControlFlowCleanupOptimizer 可是在六欲锁魂阵里的.
		class ControlFlowCleanupOptimizer : public IntraProcOptimizer 
		{
		private:
			void RemoveEntry(ControlFlowNode * node, int id)	//为什么要移除进入节点?大概是因为空的没用~
			{
				RemoveEntryAt(node->Entries, id);
				for (auto & instr : node->Code)
					if (instr.Func == Operation::Phi)	//这会还没有phi吧.. 难道这个是那之后的事吗?
						instr.Operands.erase(instr.Operands.begin() + id);
			}

			// clear cfg node and remove phi arguments
			void RemoveNode(ControlFlowNode * node, std::unordered_set<ControlFlowNode *> & frontiers, BitIntSet & removedVars)
			{
				for (auto & frontier : node->DominateFrontier)
					frontiers.insert(frontier);
				for (auto & instr : node->Code)
					if (instr.LeftOperand.IsVariable())
						removedVars.Add(instr.LeftOperand.Var->Id);
				node->Code.Clear();
				for (auto & child : node->DomChildren)
					RemoveNode(child, frontiers, removedVars);
			}
			
			//移除不可到达的分支.
			bool RemoveUnreachableBranches(ControlFlowGraph * program)
			{
				bool changed = false;
				for (auto & node : program->Nodes)
				{
					if (node.get() == program->Source || node.get() == program->Sink)
						continue;
					for (auto & instr: node->Code)
					{
						if (instr.Func == Operation::Branch)
						{
							if (instr.Operands[0].IsIntegral())
							{
								if (instr.Operands[0].IntValue && instr.Func == Operation::Branch && instr.Operands[2].IntValue == 0 ||
									(!instr.Operands[0].IntValue) && instr.Func == Operation::Branch && instr.Operands[2].IntValue == 1 )
								{
									// remove non-branching edge
									instr.Func = Operation::Jump;
									int entryId = EntryIndexOf(node->Exits[1]->Entries, node.get());
									RemoveEntry(node->Exits[1], entryId);
									node->Exits[1] = 0;
								}
								else
								{
									// remove branching edge
									instr.Func = Operation::Jump;
									int entryId = EntryIndexOf(node->Exits[0]->Entries, node.get());
									RemoveEntry(node->Exits[0], entryId);
									node->Exits[0] = node->Exits[1];
									node->Exits[1] = 0;
								}
								for (auto rdomChild : node->ReverseDomChildren)
								{
									rdomChild->ReverseDominateFrontier = node->ReverseDominateFrontier;
								}
								changed = true;
							}
						}
					}
				}
				if (changed)
				{
					BitIntSet removedVars;
					std::unordered_set<ControlFlowNode *> affectedFrontiers;
					removedVars.SetMax(static_cast<int>(program->Variables.size()));
					for (auto & node : program->Nodes)
					{
						if (node.get() == program->Source || node.get() == program->Sink)
							continue;
						if (node->Entries.empty())
							RemoveNode(node.get(), affectedFrontiers, removedVars);
					}
					for (auto & frontier : affectedFrontiers)
					{
						// update phi arguments
						for (auto & instr : frontier->Code)
						{
							if (instr.Func == Operation::Phi)
							{
								std::vector<Operand> newOperands;
								for (auto & op : instr.Operands)
								{
									if (!removedVars.Contains(op.Var->Id))
										newOperands.push_back(op);
								}
								instr.Operands.clear();
								instr.Operands.reserve(newOperands.size());
								for (auto & operand : newOperands)
									instr.Operands.push_back(operand);
							}
						}
					}
				}
				return changed;
			}
			bool Clean(ControlFlowGraph * graph)
			{
				CompactNodes(graph);
				bool result = false;	//dead里面没有这个。 对应10-2里的 Clean()算法，下面的部分对应OnePass(). 就是dead里那些，一模一样了。
				bool changed = true;
				while (changed)
				{
					changed = false;
					auto traverse = graph->GetPostOrder();
					for (auto & node : traverse)
					{
						if (node == 0)
							continue;
						if (node == graph->Source || node == graph->Sink)
							continue;
						// Step 1: Fold redundant branch
						if (node->GetExitCount() == 2 && node->Exits[0] == node->Exits[1])
						{
							node->Exits[1] = 0;
							if (node->Code.Count())
							{
								node->Code.Last().Func = Operation::Jump;
							}
							Compiler::Intermediate::RemoveEntry(node->Exits[0]->Entries, node);
							Compiler::Intermediate::RemoveEntry(node->Exits[0]->Entries, node);
							node->Exits[0]->Entries.push_back(node);
							changed = true;
						}
						if (node->GetExitCount() == 1)
						{
							// Step 2: Remove empty block
							if (node->Code.Count()==0 || node->Code.Count()==1 && node->Code.First().Func == Operation::Jump)
							{
								int entryId = EntryIndexOf(node->Exits[0]->Entries, node);
								RemoveEntryAt(node->Exits[0]->Entries, entryId);
								node->Exits[0]->Entries.insert(node->Exits[0]->Entries.begin() + entryId, node->Entries.begin(), node->Entries.end());
								for (auto & upperNode : node->Entries)
								{
									for (int j = 0; j<2; j++)
										if (upperNode->Exits[j] == node)
											upperNode->Exits[j] = node->Exits[0];
								}
								graph->Nodes[node->Id] = 0;
								node = 0;
								changed = true;
								continue;
							}
							// Step 3: merge block
							if (node->Exits[0] != graph->Sink && node->Exits[0]->Entries.size() == 1)
							{
								auto exitNode = node->Exits[0];
								node->Code.LastNode()->Delete();
								for (auto & instr : exitNode->Code)
									node->Code.AddLast(instr);
								node->Exits[0] = exitNode->Exits[0];
								node->Exits[1] = exitNode->Exits[1];
								for (auto child : node->Exits)
									if (child)
									{
										int entryId = EntryIndexOf(child->Entries, exitNode);
										child->Entries[entryId] = node;
									}
								graph->Nodes[exitNode->Id] = 0;
								auto iter = std::find(traverse.begin(), traverse.end(), exitNode);
								if (iter != traverse.end())
									*iter = nullptr;
								changed = true;
								continue;
							}
							// Step 4: hoist branch
							if (node->Exits[0]->Code.Count() == 1 && node->Exits[0]->GetExitCount() == 2)
							{
								auto exitNode = node->Exits[0];
								node->Code.Last() = exitNode->Code.Last();
								for (int k = 0; k<2; k++)
								{
									if (node->Exits[k])
									{
										Compiler::Intermediate::RemoveEntry(node->Exits[k]->Entries, node);
									}
									node->Exits[k] = exitNode->Exits[k];
									node->Exits[k]->Entries.push_back(node);
								}
								changed = true;
							}
						}
					}
					//到此为止，都跟死代码消除里一样，这里多了一个|  用来判断本次执行之后，结果是否跟以前一样.. result本身也是一个bool值。
					result = result | changed;
				}
				CompactNodes(graph);
				
				if (result)
				{
					for (auto & node : graph->Nodes)
					{
						TransformInvariant::PlacePhiAtTop(node.get());
						for (auto & instr : node->Code)
							instr.CFG_Node = node.get();
					}
					graph->ComputeDominatorTree();
				}
				return result;
			}
			//删掉随着时代变迁已经空了的节点. 这个也跟死代码消除里一样.
			void CompactNodes(ControlFlowGraph * graph)
			{
				auto traverse = graph->GetPostOrder();
				BitIntSet nodeMarks;
				nodeMarks.SetMax(static_cast<int>(graph->Nodes.size()));
				int count = 0;
				for (auto & node:traverse)
				{
					count ++;
					nodeMarks.Add(node->Id);
				}
				for (auto & node:traverse)
				{
					if (nodeMarks.Contains(node->Id))
					{
						std::vector<ControlFlowNode*> newEntries;
						for (int i = 0; i < static_cast<int>(node->Entries.size()); i++)
							if (nodeMarks.Contains(node->Entries[i]->Id))
								newEntries.push_back(node->Entries[i]);
						node->Entries.clear();
						node->Entries.reserve(newEntries.size());
						for (auto * entry : newEntries)
							node->Entries.push_back(entry);
						ControlFlowNode * keptExits[2] = { nullptr, nullptr };
						int keptExitCount = 0;
						for (int i = 0; i<2; i++)
						{
							auto * exit = node->Exits[i];
							if (exit && nodeMarks.Contains(exit->Id))
								keptExits[keptExitCount++] = exit;
						}
						node->Exits[0] = keptExits[0];
						node->Exits[1] = keptExits[1];
						if (node->GetExitCount()==1)
							node->Code.Last().Func = Operation::Jump;
					}
				}
				std::vector<std::shared_ptr<ControlFlowNode>> newNodes;
				newNodes.reserve(count);
				for (int i = 0; i < static_cast<int>(graph->Nodes.size()); i++)
				{
					if (nodeMarks.Contains(i))
					{
						graph->Nodes[i]->Id = static_cast<int>(newNodes.size());
						newNodes.push_back(graph->Nodes[i]);
					}
				}
				graph->Nodes = std::move(newNodes);
			}
		public:
			virtual ProgramOptimizationResult Optimize(std::shared_ptr<ControlFlowGraph> program) override
			{
				ProgramOptimizationResult rs;
				rs.Program = program;
				rs.Changed = RemoveUnreachableBranches(program.get());
				rs.Changed |= Clean(rs.Program.get());
				
				return rs;
			}
		};

		std::unique_ptr<IntraProcOptimizer> CreateControlFlowCleanupOptimizer()
		{
			return std::make_unique<ControlFlowCleanupOptimizer>();
		}
	}
}
