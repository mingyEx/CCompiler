#include "InterferenceAnalysis.h"
#include <memory>
#include <unordered_set>
#include <vector>

namespace Compiler
{
	namespace Intermediate
	{
		std::vector<std::shared_ptr<LiveRange>> InterferenceAnalysis::ComputeLiveRanges(ControlFlowGraph * program, int & ranges)
		{
			std::vector<std::shared_ptr<LiveRange>> rs;	//liveRange竟然是list的？
			rs.reserve(program->Variables.size());	//有多少变量就有多少live range. 那么这个玩意对等的是phi-web 或者phi-congruent 吗，区别？
			int id = 0;
			for (auto & var : program->Variables)	//每次循环都给本次的变量初始化了一个LiveRange，并且塞到了rs里。
			{
				auto range = std::make_shared<LiveRange>();
				if (var->Id != id)	//每个变量都匹配一个id 这个id用来做什么? 设置为我们的range的id,来保证可以跟对应变量相一致.
					throw InvalidProgramException(L"Inconsistent variable ids");
				range->Id = id++;
				range->VarSet.resize_for_universe(static_cast<int>(program->Variables.size()));
				range->VarSet.insert(var->Id);
				rs.push_back(range);	//编队
			}
			//给每个变量都搞了一个live range.

			//这里就是原模原样的phi - web发现算法。
			for (auto & node : program->Nodes)
			{
				for (auto & instr:node->Code)
				{
					if (instr.Func == Operation::Phi)
					{
						auto range = rs[instr.LeftOperand.Var->Id];
						for (auto & op : instr.Operands)
						{
							auto & curRange = rs[op.Var->Id];
							range->VarSet.union_with(curRange->VarSet);
							curRange = range;	
						}
					}
				}
			}

			// Deduplicate merged phi-web representatives.
			std::unordered_set<int> id_dedupe;

			std::vector<LiveRange *> dRanges;	//这个是跟前面初始化的list<range>一模一样的东西，区别是他用了裸指针，并且里面暂时还没内容。
			dRanges.reserve(rs.size());

			for (auto & range : rs)	//然后开始搬运
			{
				if (id_dedupe.insert(range->Id).second)
				{
					dRanges.push_back(range.get());
				}
			}
			for (int i = 0; i < static_cast<int>(dRanges.size()); i++)		// 下边遍历，把每个range的id都升序赋值。
				dRanges[i]->Id = i;
			ranges = static_cast<int>(id_dedupe.size());
			return rs;
		}	//没看懂怎么“计算”了，明明啥都没有啊...

		// eac书上有原文对应这段。
		InterferenceGraph InterferenceAnalysis::BuildInterferenceGraph(ControlFlowGraph * program)
		{
			InterferenceGraph rs(static_cast<int>(program->Variables.size()));
			program->ComputeVariableLiveness();	
			EnumerableIntSet liveNow;
			liveNow.SetMax(static_cast<int>(program->Variables.size()));
#ifdef _DEBUG
			// check variable id integrity 
			for (int i = 0; i < static_cast<int>(program->Variables.size()); i++)
				if (i != program->Variables[i]->Id)
					throw InvalidProgramException(L"Variable Id integrity check failed.");
#endif
			IntSet excludeSet;
			excludeSet.SetMax(static_cast<int>(program->Variables.size()));
			for (auto & node : program->Nodes)
			{
				// build live now
				liveNow.Clear();
				liveNow.UnionWith(node->LiveOut);
				for (auto iter = node->Code.LastNode(); iter; iter = iter->GetPrevious())
				{
					auto & instr = iter->Value;
					if (instr.LeftOperand.IsVariable())
					{
						int var0 = instr.LeftOperand.Var->Id;
						int varExclude = -1;
						if (instr.Func == 0 && instr.Operands[0].IsVariable())
						{
							for (auto var : liveNow.Values)
								if (var != instr.Operands[0].Var->Id)
									rs.AddEdge(var0, var);
						}
						else if (instr.Func == Operation::Phi)
						{
							excludeSet.Clear();
							for (auto op : instr.Operands)
								excludeSet.Add(op.Var->Id);
							for (auto var : liveNow.Values)
								if (!excludeSet.Contains(var))
									rs.AddEdge(var0, var);
						}
						else
						{
							for (auto var : liveNow.Values)
								rs.AddEdge(var0, var);
						}
						// update liveNow
						liveNow.Remove(var0);
					}
					for (auto & op : instr.Operands)
						if (op.IsVariable())
							liveNow.Add(op.Var->Id);
				}
			}
			return rs;
		}
	}
}
