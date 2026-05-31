#ifndef COMPILER_CFG_H
#define COMPILER_CFG_H

#include <algorithm>
#include <filesystem>
#include <memory>
#include <vector>

#include "IntermediateCode.h"
#include "BitIntSet.h"
#include "IntSet.h"

namespace Compiler
{
	namespace Intermediate
	{
		class ControlFlowNode
		{
		public:
			int Id;
			std::vector<ControlFlowNode*> Entries;
			ControlFlowNode* Exits[2];
			InstructionList Code;
		public:
			ControlFlowNode* ImmediateDominator, * ReverseImmediateDominator;
			std::vector<ControlFlowNode *> DomChildren;
			std::vector<ControlFlowNode *> ReverseDomChildren;
			std::vector<ControlFlowNode *> DominateFrontier;
			std::vector<ControlFlowNode *> ReverseDominateFrontier;
			IntSet LiveOut, LiveIn;	// IntSet来进行集合运算 
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
				operator=(std::move(node));
			}
			ControlFlowNode & operator = (ControlFlowNode && node)
			{
				Id = node.Id;
				Entries = std::move(node.Entries);
				Exits[0] = node.Exits[0]; Exits[1] = node.Exits[1];
				Code = std::move(node.Code);
				ImmediateDominator = node.ImmediateDominator;
				ReverseImmediateDominator = node.ReverseImmediateDominator;
				DomChildren = std::move(node.DomChildren);
				ReverseDomChildren = std::move(node.ReverseDomChildren);
				DominateFrontier = std::move(node.DominateFrontier);
				ReverseDominateFrontier = std::move(node.ReverseDominateFrontier);
				LiveOut = std::move(node.LiveOut);
				LiveIn = std::move(node.LiveIn);
				return *this;
			}
			InstructionNode* FirstInstruction()
			{
				InstructionNode * rs = Code.FirstNode();	
				//取出第一条指令。Code 使用稳定节点的 InstructionList；如果第一个是 phi 就继续找下一条。
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

		inline int EntryIndexOf(const std::vector<ControlFlowNode*>& entries, ControlFlowNode* node)
		{
			auto iter = std::find(entries.begin(), entries.end(), node);
			return iter == entries.end() ? -1 : static_cast<int>(iter - entries.begin());
		}

		inline void RemoveEntryAt(std::vector<ControlFlowNode*>& entries, int index)
		{
			entries.erase(entries.begin() + index);
		}

		inline void RemoveEntry(std::vector<ControlFlowNode*>& entries, ControlFlowNode* node)
		{
			auto iter = std::find(entries.begin(), entries.end(), node);
			if (iter != entries.end())
				entries.erase(iter);
		}

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
			std::vector<std::shared_ptr<ControlFlowNode>> Nodes;
			std::vector<std::shared_ptr<Variable>> Variables;
			int ParameterCount, VariableSize;	
			//很明显是形参和实参... 可是为啥要对形参区别对待？从构造来看，形参也存在上一行里，所以这里是为了区分。
			std::vector<InstructionNode *> VarDefs;
			std::vector<ControlFlowNode *> GetPostOrder();
			std::vector<ControlFlowNode *> GetPostOrderOnReverseCFG();
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
				Variables = std::move(graph.Variables);
				VarDefs = std::move(graph.VarDefs);
				ParameterCount = graph.ParameterCount;
				VariableSize = graph.VariableSize;
				Nodes = std::move(graph.Nodes);
				Source = graph.Source;
				Sink = graph.Sink;
			}
			ControlFlowNode * AddNode()
			{
				Nodes.push_back(std::make_shared<ControlFlowNode>());
				Nodes.back()->Id = static_cast<int>(Nodes.size()) - 1;
				return Nodes.back().get();
			}
			void Dump(const std::filesystem::path & fileName);
			void ConvertToSSA();
			void ComputeDominatorTree();
			void ComputeVariableLiveness();
			static std::shared_ptr<ControlFlowGraph> FromCode(const Function & code);
			void ToCode(Function & func);
		};
	}
}

#endif
