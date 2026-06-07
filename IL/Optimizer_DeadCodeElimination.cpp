#include "Optimization.h"
#include "BitIntSet.h"
#include "TransformInvariant.h"
#include <algorithm>
#include <vector>

namespace Compiler
{
	namespace Intermediate
	{
		//�������� �������� 10.2.1�������� ������ԭ��һһ��Ӧ���������Ӳ���ʹ���ٿ�~
		class DeadCodeOptimizer : public IntraProcOptimizer
		{
		private:
			//�����ˡ��ؼ��ߡ�. Ϊʲô��������ָ�p402��ԭ��.
			bool IsCritical(Instruction & instr)
			{
				return (instr.Func == Operation::Ret ||
						instr.Func == Operation::Call ||
						instr.Func == Operation::Store);
			}

			//Ѱ������ı���˵ķ���֧��߽�,��ֵ���ҷ���.
			ControlFlowNode * FindNeareastMarkedPostDom(ControlFlowNode * node, BitIntSet & nodeMarks)
			{
				auto curNode = node->ReverseImmediateDominator;
				while (!nodeMarks.Contains(curNode->Id))
				{
					curNode = curNode->ReverseImmediateDominator;
				}
				return curNode;
			}
			
			//ɾ������ʱ����Ǩ�Ѿ����˵Ľڵ�.
			void CompactNodes(ControlFlowGraph * graph)
			{
				auto traverse = graph->GetPostOrder();	//���ͼ�ĺ�������.
				BitIntSet nodeMarks;
				nodeMarks.SetMax(static_cast<int>(graph->Nodes.size()));
				int count = 0;
				for (auto & node:traverse)
				{
					count ++;
					nodeMarks.Add(node->Id);	//������ͼ�ı���·��������Ľڵ�
				}
				for (auto & node:traverse)
				{
					if (nodeMarks.Contains(node->Id))
					{
						std::vector<ControlFlowNode*> newEntries;
						for (int i = 0; i < static_cast<int>(node->Entries.size()); i++)	//����ÿ���ڵ�����н���ڵ�.
							if (nodeMarks.Contains(node->Entries[i]->Id))	//�����ڴ˽ڵ���ͼ�ı�����ʱ�������.	����ͼ�еİ�����Щ��Ϊʲô����ڡ�����ͼ��ȴ�ڽ���ڵ��������� ��Ϊ�ǴӸ��ڵ㿪ʼ���������..? ������,����ô֪����������ʲô...
								//Ŷ����Ϊ����һ���Ѿ����������ýڵ��ͼ����һЩ�ڵ��Ѿ�ʧȥ���������ڵ�Ļ����ˣ��������ɴ�����ͼ��. �����ɡ�
								//��û������Щ�յ��Ľڵ��Ƿ񱻴�node->Entries���Ƴ���.. ���ѭ����ȫ�����������ˡ�
								newEntries.push_back(node->Entries[i]);
						
						node->Entries.clear();	//Ȼ���滻����Щ.
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
						if (node->GetExitCount()==1 && node->Code.Last().Func == Operation::Branch)
							node->Code.Last().Func = Operation::Jump;	//ֻ��һ�����Ļ��͸ɴ����ó�jump.
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

			void Clean(ControlFlowGraph * graph)
			{
				CompactNodes(graph);	//�����ڵ�.Ȼ�����㷨�Ϳ�ʼ��.
				bool changed = true;
				while (changed)
				{
					changed = false;
					auto traverse = graph->GetPostOrder();
					for (auto & node : traverse)
					{
						//if (node == 0)	//Ӧ���ǿ�ָ��.
						if (node == nullptr)
							continue;
						if (node == graph->Source || node == graph->Sink)
							continue;
						// Step 1: Fold redundant branch
						if (node->GetExitCount() == 2 && node->Exits[0] == node->Exits[1])
						{
							node->Exits[1] = 0;
							if (node->Code.Count())	//�������˼Ӧ���ǡ�ָ��������Ϊ��"
							{
								node->Code.Last().Func = Operation::Jump;	//ĩβ��һ��jump.
							}
							Compiler::Intermediate::RemoveEntry(node->Exits[0]->Entries, node);	//�����˳��Ľڵ�Ľ������Ƴ���ǰ�ڵ㡣 ΪʲôҪд����?
							Compiler::Intermediate::RemoveEntry(node->Exits[0]->Entries, node);	//��Ϊ������ָ��Ķ���ͬһ���ڵ㣬���Ǹ��ڵ�������������㶼�ǵ�ǰ����Ҫȫ��ɾ��.
							node->Exits[0]->Entries.push_back(node);		//Ȼ��ѱ��ڵ�ӽ�ȥ�� node��һ�����ã�ֱ���޸ľ�����ΪʲôҪɾ����? �����Ϊ�˷�ֹ��ֻ��һ���ڵ�Ϊenter���������ʱ����.
							changed = true;
						}
						if (node->GetExitCount() == 1)
						{
							// Step 2: Remove empty block
							if (node->Code.Count()==0 || node->Code.Count()==1 && node->Code.First().Func == Operation::Jump)
							{
								//���ڿսڵ㣬������һ��ָ����Ϊjump��.

								int entryId = EntryIndexOf(node->Exits[0]->Entries, node);	//���ý���ڵ�Ϊ���ڵ����һ���ڵ�Ŀ�ʼ���Ӧ�Ĳ�λ.
								RemoveEntryAt(node->Exits[0]->Entries, entryId);		//�����?�Ƴ�entryId�ڵ�.
								node->Exits[0]->Entries.insert(node->Exits[0]->Entries.begin() + entryId, node->Entries.begin(), node->Entries.end());	//�ѽ���˽ڵ�Ķ�ת����һ����ȥ.
								for (auto & upperNode : node->Entries)	//�����? �˽ڵ�Ľ��룬���������˳��ڵ��Ƿ���ڣ�ȫ������Ϊ�˽ڵ���˳�.
									//ǰ����������ִ����������� ����Ĺ�ע�㶼�ǡ��˽ڵ�����һ�����Ĺ�ϵ���������ﴦ�����ǡ��˽ڵ���ǰ����Ĺ�ϵ����
								{
									for (int j = 0; j<2; j++)
										if (upperNode->Exits[j] == node)
											upperNode->Exits[j] = node->Exits[0];
								}
								graph->Nodes[node->Id] = 0;	//������֮��һ�ж�����.
								node = 0;
								changed = true;
								continue;
							}
							// Step 3: merge block
							//������ʲô.. �ϲ������飿 ǰ�治���Ѿ���.. Ŷ��ǰ��ֻ���ڴ����ڵ�����Щ����. ��������˵ɶ?
							if (node->Exits[0] != graph->Sink && node->Exits[0]->Entries.size() == 1)
							{
								//�����ǰ�ڵ���˳�����ͼ���˳��������˳��ڵ������Ϊ1��{��˵���ǵ��߹�ϵ����������������Ҳ���ںϲ�Node�ɣ�ǰ������ɾ��ĳ��Node,�������ںϲ�����������ָ���Node.}
								auto exitNode = node->Exits[0];
								node->Code.LastNode()->Delete();	//���һ��ָ����Ȼ����ת��ɾ����Ȼ�����һ�����������ͺ�.
								for (auto & instr : exitNode->Code)
									node->Code.AddLast(instr);
								node->Exits[0] = exitNode->Exits[0];	//�˳��ڵ���������Ϊ��һ����.
								node->Exits[1] = exitNode->Exits[1];
								for (auto child : node->Exits)	//Ȼ�����˳��ڵ�?
									if (child)
									{
										int entryId = EntryIndexOf(child->Entries, exitNode);	//�˳��ڵ������Ϊ�գ���ȡ��������һ���ڵ��Id,Ȼ��ѱ��ڵ�����Ϊ�����Ķ�Ӧλ�á� ���˶���. �ӱ��ڵ����ܵ���һ�������ӵĲ��λ�ö����ɱ��ڵ㣬ΪʲôҪִ���������? ��Ϊ����ĸ�ֵ�Ѿ��ı�����Щ��������Ҫ����������һ�ڵ㵽������ָ��.
										child->Entries[entryId] = node;
									}
								graph->Nodes[exitNode->Id] = 0;	//Ȼ�������ڵ���ͼ���λ���ÿգ�����ͻᱻ������.
								auto iter = std::find(traverse.begin(), traverse.end(), exitNode);	//������traverse���Ҳ���Ϊ0.
								if (iter != traverse.end())
									*iter = nullptr;
								changed = true;
								continue;
							}
							// Step 4: hoist branch ������֧��
							//ܳ����ֱ�ϸ����������ģ�һһ��Ӧ.
							//���Ż��������ǣ������һ����Ϊ����ֻ��������֧���Ͱѷ�֧�㵽�˽ڵ���.
							if (node->Exits[0]->Code.Count() == 1 && node->Exits[0]->GetExitCount() == 2)
							{
								auto exitNode = node->Exits[0];
								node->Code.Last() = exitNode->Code.Last();	//����һ�������ָ��������ڵ�����һ��{��Ϊ����Node�����һһ��ָ�����ת}.

								for (int k = 0; k<2; k++)	//Ȼ��ִ�д�������.
								{
									node->Exits[k] = exitNode->Exits[k];
									int entryId = EntryIndexOf(exitNode->Exits[k]->Entries, exitNode);
									exitNode->Exits[k]->Entries[entryId] = node;
								}
								changed = true;
							}
						}
					}
				}
				CompactNodes(graph);	//����Ĳ��������һЩ�սڵ㣬�ٴ�����һ��.
				
				for (auto & node : graph->Nodes)
				{
					TransformInvariant::PlacePhiAtTop(node.get());	//�����ɶ�� ��������..��ÿ���ڵ������.. ssa�Ĳ���.
					for (auto & instr : node->Code)	//�Խڵ����ÿһ��ָ��ѽڵ��ָ�붼��ֵ����ָ���Ӧ��CFG_NOde,���������Ĳ����ı���ָ�����Nodeֵ��...
						instr.CFG_Node = node.get();
				}
				graph->ComputeDominatorTree();	//Ȼ�����¼���һ��֧������.����������趨�ڵ�ĸ���ָ��.
			}

			//ɾ��ĳ���ڵ��ָ����id��Ӧ��Enter.
			void RemoveEntry(ControlFlowNode * node, int id)
			{
				RemoveEntryAt(node->Entries, id);
				for (auto & instr : node->Code)	//��������˽ڵ�����phi�Ļ�����ɾ�����������enter��id.
					if (instr.Func == Operation::Phi)
						instr.Operands.erase(instr.Operands.begin() + id);
			}
		public:
			DeadCodeOptimizer() {};
			
			//Ȼ���Ǹ��㷨����Ҫ����.
			virtual ProgramOptimizationResult Optimize(std::shared_ptr<ControlFlowGraph> program) override
			{
				ProgramOptimizationResult result;
				result.Program = program;
				result.Changed = false;
				std::vector<Instruction *> workList;	//α�������worklist��
				BitIntSet nodeMark;				//���
				nodeMark.SetMax(static_cast<int>(program->Nodes.size()));
				auto traverse = program->GetPostOrder();
				for (int i = static_cast<int>(traverse.size()) - 1; i>=0; i--)
				{
					int instrId = 0;
					for (auto & instr : traverse[i]->Code)
					{
						if (IsCritical(instr))
						{
							nodeMark.Add(instr.CFG_Node->Id);	//ֻ����ǹؼ�������ָ� �������ڱ��Node.
							instr.Mark = 1;
							workList.push_back(&instr);
						}
						else
							instr.Mark = 0;
						instrId++;
					}
				}
				while (!workList.empty())
				{
					auto &instr = *workList.back();
					workList.pop_back();
					for (auto & op : instr.Operands)
					{
						if (op.IsVariable() && program->VarDefs[op.Var->Id] != 0)
						{
							//����Ǹ����? ��ͼ��ı��������ָ��list. {����Ҳ�ǣ�forѭ������ֻ��x <- y op z���֡����塱��ָ����д���������if���С�...!=0����������ֻ��Ϊ�˻�ȡ����cfg���ֵ.}
							auto defInstr = &program->VarDefs[op.Var->Id]->Value;
							//��p403�������if def.

							if (defInstr->Mark == 0)
							{
								nodeMark.Add(defInstr->CFG_Node->Id);
								defInstr->Mark = 1;
								workList.push_back(defInstr);
							}
						}
					}
					//��ȫ��Ӧ����Ĵ���. ����ָ������Ӧ��CFG_Node��Node����list<rdf>.
					for (auto node : instr.CFG_Node->ReverseDominateFrontier) //node ���� b
					{
						/*if (node->Code.Count() == 0 ||
							node->Code.Last().Func != Operation::JumpF &&
							node->Code.Last().Func != Operation::JumpT)
							throw InvalidProgramException(L"Inconsistant CFG");*/
						auto &bInstr = node->Code.Last();	//j is the branch that ends b. j== bInstr
						if (bInstr.Mark == 0)
						{
							bInstr.Mark = 1;
							nodeMark.Add(bInstr.CFG_Node->Id);	//����һ������������,Ϊʲô��.
							workList.push_back(&bInstr);
						}
					}
				}
				//�����ӦSweep()
				for (int i = 0; i < static_cast<int>(program->Nodes.size()); i++)
				{
					int instrId = 0;
					auto & code = program->Nodes[i]->Code;
					for (auto instrNode = code.FirstNode(); instrNode != nullptr; )
					{
						auto nextInstrNode = instrNode->GetNext();
						auto & instr = instrNode->Value;
						if (instr.Mark == 0)
						{
							if (instr.Func == Operation::Branch)
							{
								result.Changed = true;
								instr.Func = Operation::Jump; //��д��jump
								// find nearest marked post dominator
								//
								auto postDom = FindNeareastMarkedPostDom(instr.CFG_Node, nodeMark);
								for (int j = 0; j<2; j++)
									if (instr.CFG_Node->Exits[j] != 0)
										Compiler::Intermediate::RemoveEntry(instr.CFG_Node->Exits[j]->Entries, instr.CFG_Node);
								//�ѷ�֧�滻��jump. ��ת���ó����ĵ�һ����������ָ��ĺ���֧���š�

								instr.CFG_Node->Exits[0] = postDom;
								instr.CFG_Node->Exits[1] = 0;
								postDom->Entries.push_back(instr.CFG_Node);
							}
							//�����ָ֧��δ����ǣ�������̽ڵ�ֱ����ֱ�ӵĺ���֧��ڵ㣬�����������ò���������ֱ��ɾ��.
							else if (instr.Func != Operation::Jump)
							{
								result.Changed = true;
								instrNode->Delete();
							}
						}
						instrNode = nextInstrNode;
					}
				}
				/*if (result.Changed)
					Clean(result.Program.Ptr());*/
				
				return result;
			}
		};
		
		std::unique_ptr<IntraProcOptimizer> CreateDeadCodeOptimizer()
		{
			return std::make_unique<DeadCodeOptimizer>();
		}
	}
}
