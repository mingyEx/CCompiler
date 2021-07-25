#ifndef COMPILER_CFG_H
#define COMPILER_CFG_H

#include "IntermediateCode.h"
#include "IntSet.h"

namespace Compiler
{
	namespace Intermediate
	{
		using namespace CoreLib::Basic;

		class ControlFlowNode
		{
		public:
			int Id;
			List<ControlFlowNode*> Entries;	
			ControlFlowNode* Exits[2];
			LinkedList<Instruction> Code;
		public:
			ControlFlowNode* ImmediateDominator, * ReverseImmediateDominator;
			List<ControlFlowNode *> DomChildren, ReverseDomChildren;		  
			List<ControlFlowNode *> DominateFrontier, ReverseDominateFrontier;
			IntSet LiveOut, LiveIn;	// 为啥这俩的类型是IntSet? 是第i个节点的id 是的。 用并查集来实现什么吗？
		public:
			ControlFlowNode()
			{
				Id = -1;
				Exits[0] = 0;
				Exits[1] = 0;
				ImmediateDominator = 0;
				ReverseImmediateDominator = 0;
			}
			ControlFlowNode(ControlFlowNode && node)
			{
				operator=(_Move(node));
			}
			ControlFlowNode & operator = (ControlFlowNode && node)
			{
				Id = node.Id;
				Entries = _Move(node.Entries);
				Exits[0] = node.Exits[0]; Exits[1] = node.Exits[1];
				Code = _Move(node.Code);
				return *this;
			}
			InstructionNode* FirstInstruction()
			{
				InstructionNode * rs = Code.FirstNode();	
				//取出第一条指令,InstructionNode是 LinkedNode<Instruction>，存了指令的一个节点
				//Code是LinkedList<Instruction>，存了指令的链表，所以后者等于List<>前者。 这里是在遍历来找东西。 当第一个是phi的时候就找下一个，因为
				//... 来看看这个函数调用的地方吧，我觉得它一定不止在程序开始节点被调用，中间遇到某个块的时候，他还是会被调用的.到那时候phi必须被跳过，因为目标机没有对应的指令 ? 再说吧~
				//上来就是outofssa,不打掉Phi就他妈的怪了..

				while (rs && rs->Value.Func == Operation::Phi)	//如果phi存在就跳过，
					rs = rs->GetNext();
				return rs;
			}

			int GetExitCount() const	//获取出口数量.
			{
				if (Exits[0] == 0)
					return 0;
				else if (Exits[1] == 0)
					return 1;
				else
					return 2;
			}
		};

		class ControlFlowGraph
		{
		private:
			void BuildDominatorTree();
			void ComputeDominateFrontier();
			void BuildReverseDominatorTree();
			void ComputeReverseDominateFrontier();
			
			static void AddEdge(ControlFlowNode * node1, int outId, ControlFlowNode * node2);
		public:
			ControlFlowNode *Source, *Sink;
			List<RefPtr<ControlFlowNode>> Nodes;
			List<RefPtr<Variable>> Variables;
			int ParameterCount, VariableSize;	//很明显是形参和实参... 可是为啥要对形参区别对待？从构造来看，形参也存在上一行里，所以这里是为了区分。
			List<InstructionNode *> VarDefs;
			List<ControlFlowNode *> GetPostOrder();
			List<ControlFlowNode *> GetPostOrderOnReverseCFG();
			ControlFlowGraph(){}
			ControlFlowGraph(const ControlFlowGraph & graph)
			{
				Variables = graph.Variables;
				VarDefs = graph.VarDefs;
				ParameterCount = graph.ParameterCount;
				VariableSize = graph.VariableSize;
				Nodes = graph.Nodes;
				Source = graph.Source;
				Sink = graph.Sink;
			}
			ControlFlowGraph(ControlFlowGraph && graph)
			{
				Variables = _Move(graph.Variables);
				VarDefs = _Move(graph.VarDefs);
				ParameterCount = graph.ParameterCount;
				VariableSize = graph.VariableSize;
				Nodes = _Move(graph.Nodes);
				Source = graph.Source;
				Sink = graph.Sink;
			}
			ControlFlowNode * AddNode()
			{
				Nodes.Add(new ControlFlowNode());
				Nodes.Last()->Id = Nodes.Count()-1;
				return Nodes.Last().Ptr();
			}
			void Dump(const String & fileName);
			void ConvertToSSA();
			void ComputeDominatorTree();
			void ComputeVariableLiveness();
			static ControlFlowGraph * FromCode(const Function & code);
			void ToCode(Function & func);
		};
	}
}

#endif