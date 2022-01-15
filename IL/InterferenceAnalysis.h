#ifndef COMPILER_INTERFERENCE_ANALYSIS_H
#define COMPILER_INTERFERENCE_ANALYSIS_H

#include "CFG.h"
#include "LibMath.h"
#include "IntSet.h"

namespace Compiler
{
	namespace Intermediate
	{
		using namespace CoreLib::Basic;

		class EnumerableIntSet	//可枚举的整数集 ，大概来自dotnet的概念
		{
		public:
			IntSet Set;	//存了俩数据结构，每次 增删都同时操作两个结构，求交时用List,参数是intset就用intset. LC上大把这类的题
			List<int> Values;
			void Add(int val)
			{
				if (!Set.Contains(val))
				{
					Values.Add(val);
					Set.Add(val);
				}
			}
			void Remove(int val)
			{
				if (Set.Contains(val))
				{
					Set.Remove(val);
					Values.FastRemove(val);
				}
			}
			void SetMax(int size)
			{
				Set.SetMax(size);
				Values.Clear();
			}
			void Clear()
			{
				Set.Clear();
				Values.Clear();
			}
			void UnionWith(const EnumerableIntSet & set)
			{
				for (auto val : set.Values)
					Add(val);
			}
			void UnionWith(const IntSet & set)
			{
				for (int i = 0; i<set.Size(); i++)
					if (set.Contains(i))
						Add(i);
			}
		};

		class LiveRange
		{
		public:
			int Id;
			IntSet VarSet;
		};

		//register interference graph: 寄存器冲突图
		class InterferenceGraph
		{
		private:
			IntSet edges;	//储存边，下面的Contains，计算对应的bits组合，如果存在这个边，就代表有一条这两个变量之间的赋值指令.
			int varCount;
		public:
			InterferenceGraph()
			{
				varCount = 0;
			}
			InterferenceGraph(int size)
			{
				edges.SetMax(size*(size+1)>>1);	// example: (5+6)/2=5 (6+7)/2=6  
				varCount = size;
			}
			InterferenceGraph(const InterferenceGraph & graph)
			{
				edges = graph.edges;
				varCount = graph.varCount;
			}
			InterferenceGraph(InterferenceGraph && graph)
			{
				edges = _Move(graph.edges);
				varCount = graph.varCount;
			}
			InterferenceGraph & operator = (const InterferenceGraph & graph)
			{
				edges = graph.edges;
				varCount = graph.varCount;
				return *this;
			}
			InterferenceGraph & operator = (InterferenceGraph && graph)
			{
				edges = _Move(graph.edges);
				varCount = graph.varCount;
				return *this;
			}
			
			void AddEdge(int var0, int var1)
			{
				int id0 = Math::Min(var0, var1);
				int id1 = Math::Max(var0, var1);
				edges.Add((id1*(id1+1)>>1) + id0);	
			}
			int AddNode()
			{
				int id = varCount;
				varCount++;
				edges.Resize(varCount*(varCount+1)>>1);
				return id;
			}
			int AddNode(const IntSet & col)
			{
				int id = varCount;
				varCount++;
				edges.Resize(varCount*(varCount+1)>>1);
				for (int i = 0; i<varCount; i++)
					if (col.Contains(i))
						AddEdge(i, id);
				AddEdge(id, id);
				return id;
			}

			inline bool Interferes(int var0, int var1)	
			{
				int id0 = Math::Min(var0, var1);
				int id1 = Math::Max(var0, var1);
				return edges.Contains((id1*(id1+1)>>1) + id0);	

			}
		};

		class InterferenceAnalysis
		{
		public:
			static List<RefPtr<LiveRange>> ComputeLiveRanges(ControlFlowGraph * program, int & ranges);
			static InterferenceGraph BuildInterferenceGraph(ControlFlowGraph * program);
		};
	}
}

#endif