#include "CFG.h"
#include "LibIO.h"
#include<iostream>
using namespace CoreLib::IO;

namespace Compiler
{
	namespace Intermediate
	{

		void ControlFlowGraph::AddEdge(ControlFlowNode * node1, int outId, ControlFlowNode * node2)
		{
			node1->Exits[outId] = node2;	
			node2->Entries.Add(node1);
		}

		ControlFlowGraph* ControlFlowGraph::FromCode(const Function & func)	//把函数里的指令塞进来cfg的node.
		{
			ControlFlowGraph *result = new ControlFlowGraph();
			ControlFlowGraph &rs = *result;
			result->Source = result->AddNode();	//此函数再创建一个新的节点，并且把当前创建的新节点指给Source。
			
			List<int> splits;	//标记下一个分叉的位置.
			
			rs.Variables.SetSize(func.Parameters.Count() + func.Variables.Count());

			for (int i = 0; i<func.Parameters.Count(); i++)
				rs.Variables[func.Parameters[i]->Id] = func.Parameters[i];	//Function里包含的变量，把对应于函数func的形参列表的第i个值的Id作为索引，取其值，以func的第i个参数覆盖之
			rs.ParameterCount = func.Parameters.Count();	//并且记录这个参数个数.

			for (int i = 0; i<func.Variables.Count(); i++)
				rs.Variables[func.Variables[i]->Id] = func.Variables[i];	//同上,不过这次是对Variables.
#ifdef _DEBUG
			for (int i = 0; i<rs.Variables.Count(); i++)
				if (rs.Variables[i] == 0)
					throw InvalidProgramException(L"Variable ids inconsistent.");
#endif
			auto code = func.Instructions.ToList();
			
			for (int i = 0; i < code.Count(); i++)	//遍历来获得操作符存在分支或者跳转的情况
			{
				if (code[i].Func == Operation::Branch || code[i].Func == Operation::Jump)
				{
					if (code[i].Func == Operation::Branch)
						splits.Add(code[i].Operands[1].IntValue);//操作数的第2个值的intValue..	//这里取得是5吗?
					else
						splits.Add(code[i].Operands[0].IntValue);	//于是if 是分支,else 就是跳转的判断.
					splits.Add(i + 1);	//只能看见buffer里的一个5
				}
			}
			splits.Add(code.Count());	//循环完了之后再记下code中所有的指令数.
			splits.Sort();				//排序之后，所有可能作为跳走和跳转目标的都有序了.

			List<int> nSplit;
			nSplit.Reserve(splits.Count());	//先保存split的元素个数?可是里面有Count(),哦，那个就当富裕了，下面一行还塞进去一个0呢。

			if (splits[0] != 0)			//排序之后,第一个就是入口节点, 如果排序之后,也就是说所有的进入节点的第一个不是第一个,就塞到nsplits里.
				nSplit.Add(0);			//并且把0作为开始节点?

			for (int i = 0; i<splits.Count(); i++)	//对于每一个splits,如果除0外的当前跳转目标不等于上一个目标,就塞进去.把整个三地址IR的每一个小框的分界线都排序号画出来
				if (i == 0 || splits[i] != splits[i-1])
					nSplit.Add(splits[i]);
			
			//下面这里在干什么呢?根据首指令来建立cfg图!
			int ptr = 0;
			int start = 0;
			auto node = rs.AddNode();	//创建新节点，在这之前rs只搞了变量
			while (ptr<nSplit.Count())
			{
				if (nSplit[ptr] == start)	//如果等于start就自增ptr
					ptr++;
				int end = nSplit[ptr];		//然后设置end为第ptr个，这样就建立了第一个基本块，并且至少是1.

				for (int i = start; i<end; i++)
				{
					code[i].CFG_Node = node;

					//node是上面的新节点, instruction code[i] 是包含CFG节点的instruct class,设定它的CFG_Node指定为当前节点
					//为什么这么做呢? code里有list<instruct>,这些instr从哪里来？ 为什么要给每个CFGNode都挂上node节点?
					//这里 start~end 是单个基本块里的所有指令，而code[i]里每一个instruct是单个函数里所有的指令 所转换成的..
					//相当于3地址代码里的每一行的列表，现在对每一行指令 设置其 CFG_Node 为本node,而下面马上把设置为本node的所有指令都塞到此node里。
					//所以我想，上面这行是不是没啥用? 还有什么地方需要知道每一条指令属于哪个节点吗？ 应该吧，比如展示的时候，当前节点所属的框图？
					//别的是不是也就没啥了，暂时留空

					node->Code.AddLast(code[i]);
					//instruction code 的第I个指令?   互相指定了一波呗,指令把自己的CFG节点指给了当前CFG的node,node把自己的Code字段添加了当前指令?
				}	
				//我的问题是，为什么.. 哦，start 和end 本来就该只差一个数字，其间的那些内容才是一个完整的基本块的所有东西。
				start = end;
				if (start < code.Count())
					node = rs.AddNode();
				else
					break;
				///单个节点构造完成,全部完了之后就把整个Node作为一个ControlFlowNode添加到ControlFlowGraph里去。
			}

			//图搞完了之后,开始遍历整个cfg里的所有节点
			for (int i = 0; i<rs.Nodes.Count(); i++)
			{
				auto node = rs.Nodes[i].Ptr();	//对于单个Node的操作.	数量大于0有啥意义? 本来也大于零吧..
				if (node->Code.Count() > 0)
				{
					if (node->Code.Last().Func == Operation::Jump)
					{	
						int id = nSplit.BinarySearch(node->Code.Last().Operands[0].IntValue) + 1;	//如果当前节点的最后一个是jump,搜索找目标地址.
													//+1 是因为，前面塞了0作为开始.
						if (id == -1)
							throw InvalidProgramException("Invalid code.");
						AddEdge(node, 0, rs.Nodes[id].Ptr());	//然后就把所有跳转到的下一个节点的边都加上了..
						node->Exits[1] = 0;						//并且该边的另一个退出就设置为0了。
					}
					else if (node->Code.Last().Func == Operation::Branch)
					{
						int id = nSplit.BinarySearch(node->Code.Last().Operands[1].IntValue)+ 1;
						if (id == -1)
							throw InvalidProgramException("Invalid code.");
						AddEdge(node, 0, rs.Nodes[id].Ptr());
						if (i+1 < rs.Nodes.Count())
						{
							AddEdge(node, 1, rs.Nodes[i+1].Ptr());
						}
					}
					else if (i+1 < rs.Nodes.Count() && node->Code.Count() > 0 && node->Code.Last().Func != Operation::Ret )
					{
						AddEdge(node, 0, rs.Nodes[i+1].Ptr());
					}
				}
			}

			//cfg构建完了，也把边都加过了.下面是?
			for (int i = 0; i<rs.Nodes.Count(); i++)
			{
				if (rs.Nodes[i]->Code.Count()==0 ||	//如果当前块里的指令数量为0
					rs.Nodes[i]->Code.Last().Func != Operation::Jump &&	//或者当前块的最后一个指令的操作符不是三个的任意一个。
					rs.Nodes[i]->Code.Last().Func != Operation::Branch &&
					rs.Nodes[i]->Code.Last().Func != Operation::Ret)
						//则，把在当前节点的指令集里加一条跳转，Operand(1) ,代表死位置吗?
					rs.Nodes[i]->Code.AddLast(Instruction(Operation::Jump, Operand(1)));
			}
			AddEdge(rs.Source, 0, rs.Nodes[1].Ptr());	//前面只是在list层次上加，现在是cfg层面的链接.
			result->Sink = result->AddNode();			//这个则是退出的位置.
			AddEdge(rs.Nodes[rs.Nodes.Count()-2].Ptr(), 0, result->Sink);	
			
			//Source 和sink分别是进入节点和退出节点 like llvm https://clang.llvm.org/doxygen/CFG_8h_source.html
			rs.ComputeDominatorTree();	//然后是支配者树和变量活跃性分析。
			rs.ComputeVariableLiveness();	//在这里的时候我没有读活跃性分析的实现吗?
			return result;
		}

		//与fromCode对应，这个应该是把cfg转换出来IR，还是汇编代码? 看到的都是机器码了,什么格式啊?
		void ControlFlowGraph::ToCode(Function & func)
		{
			func.Instructions.Clear();	//首先清空指令码链表
			auto traverse = GetPostOrder();	//对cfg的后序遍历.
			Dictionary<int, int> lineMap;
			func.VariableSize = VariableSize;
			for (int i = traverse.Count()-1; i>=0; i--)
			{
				auto node = traverse[i];	//遍历其中每一个节点
				lineMap[node->Id] = func.Instructions.Count();	//把其中的指令数量放置到lineMap里
				for (auto& instr : node->Code)	//然后把里面所有的指令都加到func的指令里面.
				{
					auto instrNode = func.Instructions.AddLast(instr);	//哦，这个是添加并返回了指向其的指针.
					if (instr.Func == Operation::Branch)
					{
						instrNode->Value.Operands[1].IntValue = node->Exits[0]->Id;	//如果是分支，就把当前cfg节点的目标点的id，赋给我们指令的操作数.
						func.Instructions.AddLast(Instruction(Operation::Jump, node->Exits[1]->Id));//可是为啥下一步不用他，而是..哦，instrNode这个返回值就是一个指针，用来赋值的，函数里传正常顺序下的下一条指令就行了.
					}
					else if (instr.Func == Operation::Jump)
					{
						instrNode->Value.Operands[0].IntValue = node->Exits[0]->Id;//这个就跟上面的一样哈哈.
					}
				}
			}
			//然后所有指令就又都回到了func里.

			int instrId = 0;
			List<int> removedInstrs;	//这是要把第i条已经移除的指令都标记下来?
			for (auto instrNode = func.Instructions.begin(); instrNode != func.Instructions.end(); ++instrNode)
			{
				auto & instr = instrNode.Current->Value;
				if (instr.Func == Operation::Jump)
				{
					int line = lineMap[instr.Operands[0].IntValue];
					if (line == instrId + 1)
					{
						instrNode.Current->Delete();
						removedInstrs.Add(instrId);
					}
					else
						instr.Operands[0].IntValue = line;
				}
				else if (instr.Func == Operation::Branch)
				{
					int line = lineMap[instr.Operands[1].IntValue];
					if (line == instrId + 1)
					{
						instrNode.Current->Delete();
						removedInstrs.Add(instrId);
					}
					else
						instr.Operands[1].IntValue = line;
				}
				instrId++;
			}
			//就猜这个是把所有到不了的指令mark出来了，细节不看。

			//下面干嘛，猜一下.. 又是跳转之类的.. 消除死代码吗，暂时不管。
			for (auto instrNode = func.Instructions.begin(); instrNode != func.Instructions.end(); ++instrNode)
			{
				auto & instr = instrNode.Current->Value;
				int * line = 0;
				if (instr.Func == Operation::Jump)
				{
					line = &instr.Operands[0].IntValue;
				}
				else if (instr.Func == Operation::Branch)
				{
					line = &instr.Operands[1].IntValue;
				}
				if (line)
				{
					int off = 0;
					for (off = 0; off < removedInstrs.Count(); off++)
						if (removedInstrs[off] > *line)
							break;
					*line -= off;
				}
				instrId++;
			}
			func.Variables.Clear();
			func.Parameters.Clear();
			for (int i = 0; i<Variables.Count(); i++)
				if (i < ParameterCount)
					func.Parameters.Add(Variables[i]);
				else
					func.Variables.Add(Variables[i]);
		}

		//搞懂了算法伪代码，自己实现也没多难。
		void ControlFlowGraph::ComputeDominatorTree()
		{
			// clear
			for (auto & node : Nodes)
			{
				node->DomChildren.Clear();
				node->DominateFrontier.Clear();
				node->ReverseDomChildren.Clear();
				node->ReverseDominateFrontier.Clear();
				node->ImmediateDominator = 0;
				node->ReverseImmediateDominator = 0;
			}
			BuildDominatorTree();
			ComputeDominateFrontier();
			BuildReverseDominatorTree();
			ComputeReverseDominateFrontier();
		}

		void TraverseGraphPostOrder(ControlFlowNode*curNode, List<ControlFlowNode*> &nodes, IntSet & visited)
		{
			if (!visited.Contains(curNode->Id))	//如果这个节点还没访问过  正常的就是塞进去完事，这个.. 哦，这个也只是在分支上多一点而已.
			{
				visited.Add(curNode->Id);	//在传进来的指针上加上
				if (curNode->Exits[0])		//如果branch分支有路径，就递归下去
					TraverseGraphPostOrder(curNode->Exits[0], nodes, visited);
				if (curNode->Exits[1])		//如果jump分支有路径，同理
					TraverseGraphPostOrder(curNode->Exits[1], nodes, visited);
				nodes.Add(curNode);			//最后再把本节点塞进去，这就算"后序遍历"了,原来图算法这么简单的吗？ 我也要玩！
			}
		}

		void TraverseReverseGraphPostOrder(ControlFlowNode*curNode, List<ControlFlowNode*> &nodes, IntSet & visited)
		{
			if (!visited.Contains(curNode->Id))
			{
				visited.Add(curNode->Id);
				for (int i = 0; i<curNode->Entries.Count(); i++)
					TraverseReverseGraphPostOrder(curNode->Entries[i], nodes, visited);
				nodes.Add(curNode);
			}
		}
		//后序遍历一个图
		List<ControlFlowNode *> ControlFlowGraph::GetPostOrder()
		{
			IntSet visited(Nodes.Count());	//所有节点都mark下，我的问题是，Nodes为啥是链表？ 因为虽然互相之间连起来了，但是储存的时候依旧是它.
			List<ControlFlowNode *> rs;	//这个用来传递? 每次 TraverseGraphPostOrder() 调用的时候,作为返回值的车车，被挂上去. 显然是后序遍历，有什么用?是在对流图执行遍历，所以后序遍历的结果应该是一棵深度优先生成树，暂且这么认为
			
			if (Nodes.Count() != 0)
				TraverseGraphPostOrder(Nodes[0].Ptr(), rs, visited);	//后两个参数用来保存状态，第一个参数，每次调用的时候?根据它的Id来判断是否访问过，并且根据其退出节点来选择下次遍历的开始节点。
			//总之，最后加入的才是当前节点的值。
			return rs;
		}
		//还是工具.
		List<ControlFlowNode *> ControlFlowGraph::GetPostOrderOnReverseCFG()
		{
			IntSet visited(Nodes.Count());
			List<ControlFlowNode *> rs;
			if (Nodes.Count() != 0)
				TraverseReverseGraphPostOrder(Nodes.Last().Ptr(), rs, visited);
			return rs;
		}

		//一个类似数组的链表，用来干嘛?
		template <typename T>
		class FakedList
		{
		public:
			int _count;
			T * _buffer;
			FakedList(int count, T* buffer)
			{
				this->_count = count;
				this->_buffer = buffer;
			}
			int Count()
			{
				return _count;
			}
			T & operator [](int idx)
			{
				return _buffer[idx];
			}
		};

		class ForwardGraphVisitor
		{
		public:
			ControlFlowNode * IntersectDominatorSet(ControlFlowNode * n1, ControlFlowNode * n2, const List<int> & order)
			{
				ControlFlowNode * ptr1, *ptr2;
				ptr1 = n1;
				ptr2 = n2;
				while (ptr1 != ptr2)
				{
					while (order[ptr1->Id] < order[ptr2->Id])	//如果p1的id小于p2的id,则p1是它自身的idom 如果order保存了节点访问的顺序，则小的代表是父节点，吗？ 这里好像没有这一说.. 暂且这么认为吧
						//如果p1的id的order比p2的小，说明p2不是p1的idom,于是p1只能是她自己
					{
						ptr1 = ptr1->ImmediateDominator;
					}
					while (order[ptr1->Id] > order[ptr2->Id])//反之，则p2的idom 只能是自己。 不对，这里p1,p2的ImmediateDominator 都是他们已经保存在作为ControlFlowNode的自己的结构体里，所以这个不是“设置为xx”，而是在重新赋值，然后这还是在循环！ 于是他们到底在干嘛呢? 在找公共的... 使得p1和p2相等的那个节点，那个节点将会是他俩共同的前向节点，对吧？
						//它会成为真正的支配节点，对于当前节点来说?
					{
						ptr2 = ptr2->ImmediateDominator;
					}
				}
				return ptr1;
			}
			inline ControlFlowNode * GetIDom(ControlFlowNode * node)
			{
				return node->ImmediateDominator;
			}
			inline void SetIDom(ControlFlowNode * node, ControlFlowNode * idom)
			{
				node->ImmediateDominator = idom;
			}
			inline void AddDomChild(ControlFlowNode * node, ControlFlowNode * child)
			{
				node->DomChildren.Add(child);
			}
			inline ControlFlowNode * GetStartNode(ControlFlowGraph & g)
			{
				return g.Nodes.First().Ptr();
			}
			inline List<ControlFlowNode *> GetPostOrder(ControlFlowGraph & g)
			{
				return g.GetPostOrder();
			}
			inline FakedList<ControlFlowNode *> GetEntries(ControlFlowNode * node)
			{
				return FakedList<ControlFlowNode*>(node->Entries.Count(), node->Entries.Buffer());
			}
			inline void AddDomFrontier(ControlFlowNode * node, ControlFlowNode * frontier)
			{
				node->DominateFrontier.Add(frontier);
			}
		};

		class ReverseGraphVisitor
		{
		public:
			ControlFlowNode * IntersectDominatorSet(ControlFlowNode * n1, ControlFlowNode * n2, const List<int> & order)
			{
				ControlFlowNode * ptr1, *ptr2;
				ptr1 = n1;
				ptr2 = n2;
				while (ptr1 != ptr2)
				{
					while (order[ptr1->Id] < order[ptr2->Id])
					{
						ptr1 = ptr1->ReverseImmediateDominator;
					}
					while (order[ptr1->Id] > order[ptr2->Id])
					{
						ptr2 = ptr2->ReverseImmediateDominator;
					}
				}
				return ptr1;
			}
			inline ControlFlowNode * GetIDom(ControlFlowNode * node)
			{
				return node->ReverseImmediateDominator;
			}
			inline void SetIDom(ControlFlowNode * node, ControlFlowNode * idom)
			{
				node->ReverseImmediateDominator = idom;
			}
			inline void AddDomChild(ControlFlowNode * node, ControlFlowNode * child)
			{
				node->ReverseDomChildren.Add(child);
			}
			inline ControlFlowNode * GetStartNode(ControlFlowGraph & g)
			{
				return g.Nodes.Last().Ptr();
			}
			inline List<ControlFlowNode *> GetPostOrder(ControlFlowGraph & g)
			{
				return g.GetPostOrderOnReverseCFG();
			}
			inline FakedList<ControlFlowNode *> GetEntries(ControlFlowNode * node)
			{
				int count = 0;
				if (node->Exits[0] != 0)
					count++;
				if (node->Exits[1] != 0)
					count++;
				return FakedList<ControlFlowNode*>(count, node->Exits);
			}
			inline void AddDomFrontier(ControlFlowNode * node, ControlFlowNode * frontier)
			{
				node->ReverseDominateFrontier.Add(frontier);
			}
		};

		//这个是eac p.392 9-24里，修改后的迭代支配者算法
		template <typename GraphVisitor>
		void BuildDominateTree_Internal(ControlFlowGraph & graph, GraphVisitor visitor)
		{
			if (graph.Nodes.Count() == 0)return;

			IntSet processed(graph.Nodes.Count());

			List<ControlFlowNode*> traverse = visitor.GetPostOrder(graph);









			//后序遍历，先挂所有子节点再挂自己,导致的结果是根节点比子节点的index大

			List<int> order;
			order.SetSize(graph.Nodes.Count());	//用来记录顺序

			for (int i = 0; i<traverse.Count(); i++)
				order[traverse[i]->Id] = i;		
			//比如说traverse[0]是第5个节点，则order里的第5个位置记录为0,代表“order里的顺序就是节点的顺序，里面的值就是排序之后节点id的值。
			//换言之，遍历order,取其中元素的值作为Id来找Node,order的顺序就是Node的顺序。

			auto startNode = visitor.GetStartNode(graph);	//开始节点直接从start获得。

			processed.Add(startNode->Id);	//把开始节点 mark 为已处理.

			visitor.SetIDom(startNode, startNode);	//设置自己为自己的Idom .不过不同书里，idom是否包括自己是不一样的，暂时不管，不重要.

			bool changed = true;
			while (changed)
			{
				changed = false;	//第一次进入就关掉

				for (int i = traverse.Count()-1; i>=0; i--)	//倒序遍历刚刚后序遍历的链表(挺好，这样dfn数组的从小到大就恰好满足了.)
				{
					auto curNode = traverse[i];			//记录本次循环的节点
					if (curNode == startNode) continue;	//如果是开始节点就跳过
					ControlFlowNode * newIdom;	//下一个idom节点，估计一定会找... 怎么确定呢？ 多个分支的共有？是的，从此节点的进入节点们挨个找，如果能发现他们的交集，那个就是了。

					int firstProceedPrecedant = -1;		//第一个处理的前向节点? 且不到build tree呢(不过此函数一定会完成这个，下个就是计算了)

					auto entries = visitor.GetEntries(curNode);	//果然，获得此节点的所有进入节点.

					//当前节点的所有进入节点，下面估计是计算支配边界的.
					//别傻逼了，下面是计算交集来获得idom的，支配边界您等到下个函数去吧...
					
					for (int j = 0; j<entries.Count(); j++)		//遍历所有当前节点的进入节点的指针
					{
						if (processed.Contains(entries[j]->Id))	
							//注意到 processed 一开始就把开始节点塞进去了。
							//如果已经处理过了(这个处理跟访问还不是一回事，等价的是书上的...? 哪个算法？显然是多遍进入的那个.)
							//如果哎没有处理过呢？ 哪里负责"处理过"这个过程?
							//暂猜"处理过"的意思是，从其他路径到达过这里，现在又来了，所以此节点必然是最近的一个idom
							//不是吧，需要所有的路径都从这里过，才可以叫"必经节点". 算了先不管，我觉得第一次很大概率此for循环根本不会执行，不信看看！
							//不是的，每次执行都会摸一个节点设置一下然后直接跳出. 那么下一次进入的时候，难道j还是0吗？
							//我不信，我觉得多试几次，这个for一定会需要多转几圈才执行这个if的，试一试！
							//艹他妈的，为什么每次都是“直接插进来，射完就滚”啊！！！ 那这个for还有蛋的意义啊？意义是...
							//还有，这个processed是啥时候被塞进去的节点？ 是不是一开始塞满了，后面往外弹? 弹多了就if失误了?
							//不是，一开始没有节点，每次循环完针对每个节点的for之后才会塞进去一个.
							//那他妈的为什么这么邪门，每次都能存在? 而且每次都把new设置为entries[0],不鸡巴累吗?
							//哦，entries外面的curNode每次循环都在变...
							//那又如何呢?
							//那他妈的这个for干了几把啥... 每次都他妈的设置一个就break. 艹！?
							//相比于下一个循环，就只负责了执行"是否处理过"这个检查？ 而且他妈个逼的为啥每次都是true?
							//您有fail的时候吗？ 要不我直接取消检查，上来就设置得了？？？？？
						{
							newIdom = entries[j];
							//然后就将其设置为新idom节点.	
							//哦，如果已经来过一次当前节点，下次从别的进入分支又看到这个了，则说明这个是站在十字路口的男人？
							//我觉得大概是这样..

							firstProceedPrecedant = j;	//把本次处理的前向节点设置为j. 这个j是在标记本节点的第几个是idom？
							break;
						}
					}
					for (int j = 0; j<entries.Count(); j++)		//同样的循环是在干嘛? 如果此次j等于上次循环的j,就继续，否则？
					{
						if (j == firstProceedPrecedant)			//这里的j应该是第j个块，如果这个块等于 那个玩意.. 就跳过，意思是？
							continue;
						if (visitor.GetIDom(entries[j]))		//否则获得当前块的直接支配节点，为什么呢?
							newIdom = visitor.IntersectDominatorSet(newIdom, entries[j], order);	
						//并且计算 相交支配集，这个交集应该就是在模拟找两个节点的公共idom
						//如果第一个节点的ID小于第二个，就找它的idom，否则就设置第二个的为第二个的自己。这样找到的是公共父节点.
					}
					//然后呢? 如果当前节点的idom不是刚刚计算出来的那个(遍历curNode所有front所找到的公共的)，就把它设置成idom(唯一)
					if (visitor.GetIDom(curNode)!= newIdom)		//我的问题是，到这里的时候，这个newIdom是翻遍了curNode所有entries吗?看起来是的，上一个for把所有entry节点的idom都翻了一遍得到的这个newIdom.

						//如果当前节点的idom就是等于newIdom了，就代表？计算已经收敛，changed保持为false,跳出当前循环。
						//否则则如下，设置新节点，并且改为true,这样此while()就会继续。
						//不过下次继续的时候，跟上一次不同的地方在于,curNode的idom变了，如此而已。
						//代码的执行过程我了解了，也知道这么计算到底能计算出个啥了。
						//但是伪代码是怎么刻画这些集合的，晚点再说.
					{
						visitor.SetIDom(curNode, newIdom);
						changed = true;
					}
					processed.Add(curNode->Id);	//在最外层，每次循环塞一个当前节点.
				}
			}
			visitor.SetIDom(startNode, 0);		//把起始节点的idom设置为0

			for (int i = 0; i<graph.Nodes.Count(); i++)						//然后这个遍历，获得当前... 图的某一个结点指针的idom之后， 
				if (visitor.GetIDom(graph.Nodes[i].Ptr()))					//对于每个节点，摸到其idom，然后把idom的后面挂上树.
					visitor.AddDomChild(visitor.GetIDom(graph.Nodes[i].Ptr()), graph.Nodes[i].Ptr());	


			//把当前节点的idom和当前节点都拿出来，把后者挂成前者的子节点，是list哦~ 这是在建树吗.. 看起来是的，不过所有的孩子拴在一根葫芦藤上，而不是单独插入到idom里.
		}
		//所以 这一整个过程只是把这个idom 树建起来而已，虽然称为树，但是人家没有严格按照树的定义，dfn也恰好反着的.
		
		//下面计算支配边界! frontier 边界
		template <typename GraphVisitor>
		void ComputeDominateFrontier_Internal(ControlFlowGraph & graph, GraphVisitor visitor)
		{
			for (int i = 0; i<graph.Nodes.Count(); i++)		//对于所有节点 
			{
				auto curNode = graph.Nodes[i].Ptr();
				auto entries = visitor.GetEntries(curNode);
				if (entries.Count() > 1)
				{
					for (int j = 0; j<entries.Count(); j++)	//遍历每个节点的进入点
					{
						auto p = entries[j];
						while (p && p != visitor.GetIDom(curNode))	//若此进入点存在且不为当前节点的idom	 
							//GetIDom()  : return curNode->ImmediateDominator;
						{
							visitor.AddDomFrontier(p, curNode);		//把curNode设置为 p 的DominateFrontier (支配边界)
							//为什么呢？根据定义，边界是指"恰好不是dom的那个节点",如果一个节点的进入点的不是他的idom,说明...?
							//说明还有其他的路径可以进入此节点，则此进入节点不支配此节点，于是对于此进入节点来说，当前节点既是它的支配边界. 
							p = visitor.GetIDom(p); //把p重设为p的idom. 既沿着进入节点的idom向上找，如果p依旧不是curNode的idom,就再把curNode设置为p->idom的支配边界? 是的，看图你就知道，只要满足 p != visitor.GetIDom(curNode)，那么这些p都恰好无法支配curNode,且能支配p(毕竟是沿着idom找过去的),于是可以确定这些p的支配边界。
						}//由于这个循环是从哪里开始的? 针对每个节点都执行一遍，所以...?找到的就是全部！
					}
				}
			}
		}

		//然后是主菜
		void ControlFlowGraph::BuildDominatorTree()
		{
			BuildDominateTree_Internal(*this, ForwardGraphVisitor());
		}

		void ControlFlowGraph::ComputeDominateFrontier()
		{
			ComputeDominateFrontier_Internal(*this, ForwardGraphVisitor());
		}

		void ControlFlowGraph::BuildReverseDominatorTree()
		{
			BuildDominateTree_Internal(*this, ReverseGraphVisitor());
		}

		void ControlFlowGraph::ComputeReverseDominateFrontier()
		{
			ComputeDominateFrontier_Internal(*this, ReverseGraphVisitor());
		}
		//这四个都是转发，知道干了啥就好.

		//这些是下一阶段的事，不影响当前窗口
		void ControlFlowGraph::ComputeVariableLiveness()
		{
			int totalVars = Variables.Count();
			List<IntSet> liveIn, kill;	//由set组成的两个list. 分别存放liveIn和kill 前者代表当前块中活跃的，后者代表什么时候被杀死的?
			liveIn.SetSize(Nodes.Count());	//都初始化为节点数量，为啥，哦，list个Nodes. 每个inset里面放置的是本块里的变量.
			kill.SetSize(Nodes.Count());
			// compute initial set
			for (int i = 0; i<Nodes.Count(); i++)		//计算初始集。
			{
				Nodes[i]->LiveOut.SetMax(totalVars);	//把第i个节点，等等，为什么是total.. 因为VariableSize术语cfg,嗯！cfg代表的是一个函数，函数里的变量个数在此函数的cfg的所有node都一样多，好的。

				Nodes[i]->LiveIn.SetMax(totalVars);		//这个同上.
				liveIn[i].SetMax(totalVars);	//这俩则是我们自己定义的.
				kill[i].SetMax(totalVars);

				//这里执行的类似 图8.4的左边
				auto& code = Nodes[i]->Code;	//然后获得此节点里所有的...指令? 是的!
				code.ForEach([&](const Instruction & instr)		//遍历此code list里每一个元素，对其做以下操作:
				{
					for (int k = 0; k<instr.Operands.Count(); k++) //这个for是在遍历此 Instruction 里 List<Operand> 的.
					{
						if (instr.Operands[k].Type == OperandType::Variable		//如果当前操作数的类型是变量
							&& !kill[i].Contains(instr.Operands[k].Var->Id))	// 并且本list 里的当前 Intset kill里没有包含此变量的id
							liveIn[i].Add(instr.Operands[k].Var->Id);			//就将其加到liveIn里。
					}										//对应到书上就是，如果 三本书上都没有，我自己理解算了！
					//如果变量在此节点内没有被kill,就把他塞到liveIn里，至于啥时候会被kill,待会再说
					if (instr.LeftOperand.Type == OperandType::Variable)
						kill[i].Add(instr.LeftOperand.Var->Id);		//如果左操作数是变量，说明此变量被重新赋值了，则在当前块里重生，于是之前的不会传递到下一个里，所以标记为kill.	 其他的，再说。
				});
			}
			// for 循环完毕，这个循环是，对所有节点里的每一个指令list进行的

			if (Nodes.Count()>0)
				Nodes[0]->LiveIn = liveIn[0];	//不过上面的计算是把内容都塞到自己定义的list里了，这里先把0的塞到node的里. 下面估计会都塞进去?

			// iterative solve, in reversed post-order of reserve cfg
			// 迭代求解,以 逆后序 对保存了的cfg进行操作.
			//LiveOut(node) = Union { node.Succeeds sNode, LiveIn(sNode)+(LiveOut(sNode)-Kill(sNode)) }
			// 
			//当前节点的LiveOut是当前节点的下一个节点的LiveIn和此节点的LiveOut减去kill
			//这个好眼熟啊，我好像看过了，如何判断一个变量是活跃的呢？ 如果此节点中的变量到了下一个节点里依旧存在(liveIn),并且下一个节点出去的时候的那些里去除被重新赋值的那些，就是从这个节点成功流入到下一个节点的所有变量。
			//来点正式的说法:
			//liveout 是指，从本 node (就是基本块)退出时候，依旧活跃着的所有变量。
			//它的计算方式是，一个全局数据流分析，用于在编译时推导运行时值的流动，
			//LiveOut(node) = Union{ node.Succeeds sNode, LiveIn(sNode) + (LiveOut(sNode) - Kill(sNode)) }
			//一个后继节点的liveOut,还有,没有被kill的那些。 
			// 这里的 (LiveOut(sNode) - Kill(sNode)) 就是kill的取补集(已确认)。
			//为什么是没有被杀死的?
			//liveIn 是指，对某变量名字的第一次使用，引入了在进入b之前计算的一个值， 换言之，这个名字是上个块里传来的。
			//因为liveOut是利用succ来定义的，所以该联立的方程描述了一个反向数据流问题。
			//liveOut是n的各个后继节点m中程序入口处活动的变量的并集，。。。那liveIn与liveOut& (!kill)的关系呢?
			//前者是进入之后直接被使用的，然后被重新定义(kill),后者是一直没有被重新定义的(!kill),并且还流向了下一个图。
			//这两者有交叉的的地方吗？ 我觉得灭有。
			//这两者构成所有的liveOut? 我觉得是的！

			auto traverse = GetPostOrderOnReverseCFG();	//因为是反向数据流问题，所以这里直接跟build tree一样遍历一下。
			bool changed = true;
			IntSet newLiveOut(totalVars);//, partialLiveOut(totalVars);
			
			//这里执行的类似 图8.4的右边
			while (changed)
			{
				changed = false;
				for (int i = traverse.Count()-1; i>=0; i--)
				{
					newLiveOut.Clear();	//设置为空集，不过不同于源码里给每个LiveOut()设置为空集，这里只搞了一个，然后每次用的时候都重新计算它. 跟原版算法不大一样.
					auto node = traverse[i];
					for (int j = 0; j<2; j++)
					{
						if (node->Exits[j] == 0)
							break;
						IntSet & childLiveIn = node->Exits[j]->LiveIn;//上面已经把每个节点的LiveIn都塞进去了，这里获得引用单独摸来
						childLiveIn.Clear();	//又是清空?

						//这些是在recompute liveOut.	就差这三和函数了！
						//LiveOut(node) = Union{ node.Succeeds sNode, LiveIn(sNode) + (LiveOut(sNode) - Kill(sNode)) }
						// 现在才想起来，参数3是kill 啊！ 于是这个显然就是在求差集！ 至于为啥，待会仔细看看看！
						//这个函数等价于  (LiveOut(sNode) - Kill(sNode))
						IntSet::Subtract(childLiveIn, node->Exits[j]->LiveOut, kill[node->Exits[j]->Id]);	//不管函数了，结果在para1.

						//这个函数则是 LiveIn(sNode) +  的部分！请注意，这里的+-都是集合运算！
						childLiveIn.UnionWith(liveIn[node->Exits[j]->Id]);	//把参数塞到

						//这个函数则是最外面那个Union{}！
						newLiveOut.UnionWith(childLiveIn);
					}
					if (node->LiveOut != newLiveOut)	//这个是右边下的倒数第三行.
					{
						node->LiveOut = newLiveOut;
						changed = true;
					}
				}
			}
		}
		//我好像找到原因了，书上和搜来的都是最简单的那个构造方法，而这个要么是变体，要么是更复杂一点的。
		//去找找半精简的。
		void SSA_PlacePhiFunctions(List<RefPtr<ControlFlowNode>> & Nodes, List<RefPtr<Variable>> & Variables)
		{
			List<IntSet> phiSet, lastPhiSet;
			
			lastPhiSet.SetSize(Nodes.Count());
			bool changed = true;
			while (changed)
			{
				changed = false;
				phiSet.SetSize(Nodes.Count());	//phiSet有跟Nodes一样多的位置
				for (int i = 0; i<phiSet.Count(); i++)
					phiSet[i].SetMax(Variables.Count());	//每个phiSet里的大小是变量个数.
				for (int i = 0; i<Nodes.Count(); i++)
				{
					IntSet contribution(Variables.Count());		//大小设置为当前Node里变量的个数
					for (auto & instr : Nodes[i]->Code)		
						//遍历所有节点，中的每条指令，如果左操作数是一个变量就代表是赋值，塞到 contribution 里，等价于伪代码里的 worklist
						if (instr.LeftOperand.IsVariable())
							contribution.Add(instr.LeftOperand.Var->Id);

					contribution.IntersectWith(Nodes[i]->LiveOut);		//计算与当前Node的liveOut的交集，为的是，让每次的contribution都只保存当前Node的变量吗？ 这个交集操作赋值给谁，做了什么， contribution 的作用域又是啥？

					for (int j = 0; j<Nodes[i]->DominateFrontier.Count(); j++)	//遍历当前节点的每一个支配边界里的节点
					{
						phiSet[Nodes[i]->DominateFrontier[j]->Id].UnionWith(contribution);//取其Id,与 contribution 计算交集？ 这是在干啥?
						//至今为止，contribution存放的是当前块中可能是全局变量的变量集合， 这里的DominateFrontier则是此节点的支配边界，也就是
						//变量可能会被重新赋值的地方，求并集，有什么用？就是伪代码里最后一行...  DF(b)
						//phiSet里面的每个节点的支配边界都走一遍，跟当前contribution做并集运算，得到的结果存放在?phiSet里面，contribution只存放当前Node里的元素.
						//于是这个求并集的结果到底是什么呢？ DF(b)∪ worklist. 显然。
					}	//此计算针对所有节点，在每次while()循环里进行，所以，就跟忘了哪个一样，这个函数其实是对应代码的打乱顺序版？
				}
				for (int i = 0; i<Nodes.Count(); i++)
				{
					phiSet[i].IntersectWith(Nodes[i]->LiveIn);//LiveIn ==UEVar. 所以这里是在做.. 但是书上没有任何地方涉及到交集运算啊，全都是并集..
				}
				for (auto & node : Nodes)	//既然说到”每一个“，那phiSet跟node的的对应关系是什么样的?
				{
					if (node->Entries.Count() <= 1)
						continue;
					auto & set = phiSet[node->Id];	//每一个节点的id在phiSet里的索引，设置为set	
					for (int j = 0; j<Variables.Count(); j++)	//这个循环处理本次的所有变量，变量本身是一个 List，是本次传进来的所有变量.. 传进来的是什么呢？是本cfg的所有变量.? 这些东西不是该在cfgNode里吗？  哦，Node只有 Instruction ，没有Variables。
					{
						if (set.Contains(j) && !lastPhiSet[node->Id].Contains(j))	//所以所有变量都在整个cfg里，如果本Node里包含j并且上次没有.
						{
							List<Operand> operands;		//就说明什么？ 这次while循环塞进来了新的跨块变量？ 接下来做了啥?
							operands.SetSize(node->Entries.Count());	//把本节点的所有进入点个数都塞进来.

							for (int k = 0; k<node->Entries.Count(); k++)	//这里都是k,为什么下面是j呢？ 意指只想循环这么多遍？哦，前面是k. 但是每次都是同一个有什么意义啊?"我塞了这么多个",待会挨个赋值”这样？ 每个变量应该记录了自己的来源.. 哦，这个k就编码了Entries的来源. 那，里面的值呢？
								//这个operands[k]里的东西会被如何使用？

								operands[k] = Operand(Variables[j].Ptr());		//并且把当前变量赋值给每一个操作数？然后加一个phi函数在这里...
							//又是不会执行到的？ 可能是我的例子太简单了吧.. 暂时不管，继续前进.
							
							//这里在做什么呢？ 循环过程中如果发现了新的，之前不存在的变量出现了，就把有机会进入到此Node的所有进入点都塞到一个operands里并且..

							//往Instruction list里的开头加一条指令，指令内容是，一个phi操作和一个operands，这个operands里面是？
							auto instrNode = node->Code.AddFirst(Instruction(Operand(Variables[j].Ptr()), Operation::Phi, _Move(operands)));
							//在当前Node节点的指令开头插入一个phi函数
							instrNode->Value.CFG_Node = node.Ptr();	//这个大概是节点挂接.
							changed = true;	//这里设置为true的意义是啥？ 为了下次能够继续循环。 这里执行了哪些操作？什么时候会设置为true.要看上面。
							//这个if分支的要求是， set是... 暂且假定phiSet是”所有存在全局变量的Node,也就是“所有需要插入phi的Node".我们已经知道last跟phiset没有任何区别，纯粹就是为了保存计算结果而存在的。
							//然后这里的判断条件是，如果本节点里
						}
					}
				}
				if (changed)	//如果change被设置为true,就说明while还要继续，于是把phiSet里所有东西都丢给Last. 作为保存，下次就.. 对比？
					//它是怎么判断是否会改变的？ 返回的又是什么？
					for (int i = 0; i<Nodes.Count(); i++)
					{
						lastPhiSet[i] = _Move(phiSet[i]);
					}
			}
		}

		struct SSA_RootVar	//这是那个辅助栈，我知道！
		{
			Variable * Var; // original variable
			List<Variable * > VariableStack; // renamed variable stack for this var
			int Counter;
			SSA_RootVar()
			{
				Var = 0;
				Counter = 0;
			}
		};

		//给每个变量重新命名！加标号！
		inline Variable * SSA_NewName(Variable * var, ControlFlowGraph * graph, List<SSA_RootVar*> & rootVarMapping)
		{
			Variable * nvar = new Variable();
			nvar->Id = graph->Variables.Count();
			graph->Variables.Add(nvar);
			nvar->Name = var->Name;
			nvar->Size = var->Size;
			auto rootVar = rootVarMapping[var->Id];
			nvar->Version = rootVar->Counter;
			rootVar->Counter++;
			rootVar->VariableStack.Add(nvar);
			rootVarMapping.Add(rootVar);
			return nvar;
		}
		//然后替换?
		inline void SSA_ReplaceVar(Variable*& var, List<SSA_RootVar*> & rootVarMapping)
		{
			var = rootVarMapping[var->Id]->VariableStack.Last();
		}

		void SSA_Rename(ControlFlowNode * node, ControlFlowGraph * graph, List<SSA_RootVar*> & rootVarMapping)
		{
			// rename instructions
			for (auto iter = node->Code.begin(); iter != node->Code.end(); iter++)
			{
				auto & instr = iter.Current->Value;
				if (instr.Func != Operation::Phi)
				{
					for (auto &op : instr.Operands)
					{
						if (op.IsVariable())
						{
							SSA_ReplaceVar(op.Var, rootVarMapping);
						}
					}
				}
				if (instr.LeftOperand.IsVariable())
				{
					instr.LeftOperand.Var = SSA_NewName(instr.LeftOperand.Var, graph, rootVarMapping);
					graph->VarDefs.Add(iter.Current);
				}
			}
			// update phi functions for child vertices
			for (auto &childNode : node->Exits)
			{
				if (!childNode)
					break;
				int entryId = childNode->Entries.IndexOf(node);
				if (entryId == -1)
					throw InvalidProgramException(L"Inconsistent control flow graph.");
				for (auto & instr : childNode->Code)
				{
					if (instr.Func != Operation::Phi)
					{
						break;
					}
					SSA_ReplaceVar(instr.Operands[entryId].Var, rootVarMapping);
				}
			}
			// recursively process dominate children
			for (auto domChild : node->DomChildren)
				SSA_Rename(domChild, graph, rootVarMapping);
			// restore variable stack
			for (auto &instr : node->Code)
			{
				if (instr.LeftOperand.IsVariable())
				{
					auto & stack = rootVarMapping[instr.LeftOperand.Var->Id]->VariableStack;
					stack.RemoveAt(stack.Count()-1);
				}
			}
		}

		void ControlFlowGraph::ConvertToSSA()
		{
			SSA_PlacePhiFunctions(Nodes, Variables);	//传入的是什么东西？Variables属于ControlFlowGraph，本Nodes的
			if (Nodes.Count() > 0)		//这个函数把一堆opands跟phi一起塞到了相关节点的开头的phi指令里，但是那几个参数好像都赋值成了同一个，这里是会重新计算每一个吗?
			{
				// intialize mapping between variable aliases and original variables
				List<SSA_RootVar> rootVars;
				rootVars.SetSize(Variables.Count());
				for (int i = 0; i<Variables.Count(); i++)	//看起来这个只处理变量本身，没有涉及上述... 只好找输入输出自己想了
				{
					rootVars[i].Var = Variables[i].Ptr();	//root里每一个变量都赋值为vars里的对应的地址..
					rootVars[i].VariableStack.Add(Variables[i].Ptr());	//它里面的varStack都增加上此vars的地址？
					VarDefs.Add(0);	//这个是啥,属于ControlFlowGraph 的成员.
				}
				List<SSA_RootVar*> rootVarMapping;
				rootVarMapping.Reserve(Variables.Count()*2);	//why *2?
				for (int i = 0; i<Variables.Count(); i++)
					rootVarMapping.Add(&rootVars[i]);
				// intialize VarDef
				SSA_Rename(Nodes[0].Ptr(), this, rootVarMapping);
			}
		}
		
		//写入的有的是指令本身，有的是数字，反正知道自己以什么顺序写入的，cfgView里按照顺序读取就好了.
		//我想加个Dom[],但是没有计算这个，放弃.
		void ControlFlowGraph::Dump(const String & fileName)
		{
			BinaryWriter writer(new FileStream(fileName, FileMode::Create));
			StringBuilder sb;
			writer.Write(Nodes.Count());
			for (int i = 0; i<Nodes.Count(); i++)
			{
				writer.Write(Nodes[i]->Id);	//写入的是数字
				sb.Clear();
				for (auto& instr : Nodes[i]->Code)
				{
					sb.Append(instr.ToString());	
					sb.Append(L'\n');				
				}
				writer.Write(sb.ToString());
				for (int j = 0; j<2; j++)
				{
					if (Nodes[i]->Exits[j])
						writer.Write(Nodes[i]->Exits[j]->Id);
					else
						writer.Write(-1);
				}
				if (Nodes[i]->ImmediateDominator)
					writer.Write(Nodes[i]->ImmediateDominator->Id);
				else
				{
					writer.Write(-1);
				}
				writer.Write(Nodes[i]->DominateFrontier.Count());
				for (int j = 0; j<Nodes[i]->DominateFrontier.Count(); j++)
					writer.Write(Nodes[i]->DominateFrontier[j]->Id);
				writer.Write(Nodes[i]->DomChildren.Count());
				for (int j = 0; j<Nodes[i]->DomChildren.Count(); j++)
					writer.Write(Nodes[i]->DomChildren[j]->Id);
				if (Nodes[i]->ReverseImmediateDominator)
					writer.Write(Nodes[i]->ReverseImmediateDominator->Id);
				else
				{
					writer.Write(-1);
				}
				writer.Write(Nodes[i]->ReverseDominateFrontier.Count());
				for (int j = 0; j<Nodes[i]->ReverseDominateFrontier.Count(); j++)
					writer.Write(Nodes[i]->ReverseDominateFrontier[j]->Id);
			}
		}
	}
}