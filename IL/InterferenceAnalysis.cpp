#include "InterferenceAnalysis.h"

namespace Compiler
{
	namespace Intermediate
	{
		List<RefPtr<LiveRange>> InterferenceAnalysis::ComputeLiveRanges(ControlFlowGraph * program, int & ranges)
		{
			List<RefPtr<LiveRange>> rs;	//liveRange竟然是list的？
			rs.Reserve(program->Variables.Count());	//有多少变量就有多少live range. 那么这个玩意对等的是phi-web 或者phi-congruent 吗，区别？
			int id = 0;
			for (auto & var : program->Variables)	//每次循环都给本次的变量初始化了一个LiveRange，并且塞到了rs里。
			{
				auto range = new LiveRange();
				if (var->Id != id)	//每个变量都匹配一个id 这个id用来做什么? 设置为我们的range的id,来保证可以跟对应变量相一致.
					throw InvalidProgramException(L"Inconsistent variable ids");
				range->Id = id++;
				range->VarSet.SetMax(program->Variables.Count());	//这里把变量数量都..?放到自己的位运算计算集合里。 大概是为了待会那啥..? 静观.
				range->VarSet.Add(var->Id);		//为什么是加完别人再加上自己，这个加法是啥?把可以代表这个id值的某一位设置为1？ 我想单独把这个IntSet拿出来摸一遍。
				rs.Add(range);	//编队
			}
			//给每个变量都搞了一个live range.

			//这里就是原模原样的phi - web发现算法。
			for (auto & node : program->Nodes)
			{
				for (auto & instr:node->Code)
				{
					if (instr.Func == Operation::Phi)
					{
						RefPtr<LiveRange> range = rs[instr.LeftOperand.Var->Id];
						for (auto & op : instr.Operands)
						{
							auto & curRange = rs[op.Var->Id];
							range->VarSet.UnionWith(curRange->VarSet);
							curRange = range;	
						}
					}
				}
			}

			HashSet<int> idDedupe;	//id Dedupe ,大概是“如果不相交就可以合并”的意思吧？还是用来做什么的呢？ 也可能是 推论 的意思。
			//Dedupe - De-duplicate 看起来就是这个意思.

			List<LiveRange *> dRanges;	//这个是跟前面初始化的list<range>一模一样的东西，区别是他用了裸指针，并且里面暂时还没内容。
			dRanges.Reserve(rs.Count());

			for (auto & range : rs)	//然后开始搬运
			{
				//if 里面是 AddIfNotExists，true代表着not exists. 那么这里还往dRanges添加干嘛? 
				if (idDedupe.Add(range->Id))	//添加怎么可能失败呢? 哦，true代表“没有，且我插入了”。
				{
					dRanges.Add(range.Ptr());	//然后就要给这个list裸指针的dRanges 加上此指针. dRanges 这个东西本来只是有空间，现在的是加入，嗯。 把指针塞进来了。顺序应该还是跟那啥一样，区别是过滤了几个“已经在其中”的值。
				}
			}
			for (int i = 0; i<dRanges.Count(); i++)		// 下边遍历，把每个range的id都升序赋值。
				dRanges[i]->Id = i;
			ranges = idDedupe.Count(); //这里计算..过滤之后？ 不是，是 idDedupe 里添加了多少值。 嗯... 这个是否添加的标准就是“里面塞了几个”，算了，好像没有看到明显的“判断标准”，而且这个字典是在干啥我也不知道..
			return rs;
		}	//没看懂怎么“计算”了，明明啥都没有啊...

		// eac书上有原文对应这段。
		InterferenceGraph InterferenceAnalysis::BuildInterferenceGraph(ControlFlowGraph * program)
		{
			InterferenceGraph rs(program->Variables.Count());
			program->ComputeVariableLiveness();	
			EnumerableIntSet liveNow;
			liveNow.SetMax(program->Variables.Count());
#ifdef _DEBUG
			// check variable id integrity 
			for (int i = 0; i<program->Variables.Count(); i++)
				if (i != program->Variables[i]->Id)
					throw InvalidProgramException(L"Variable Id integrity check failed.");
#endif
			IntSet excludeSet;
			excludeSet.SetMax(program->Variables.Count());
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