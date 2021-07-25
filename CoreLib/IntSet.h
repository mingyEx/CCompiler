#ifndef BIT_VECTOR_INT_SET_H
#define BIT_VECTOR_INT_SET_H

#include <memory.h>
#include "List.h"
#include "LibMath.h"
#include "Common.h"
#include "Exception.h"

#include<iostream>
namespace CoreLib
{
	namespace Basic
	{
		//为了方便对集合进行计算的类型,用位的状态表示某个数字是否在集合中.
		//todo: 解释清楚位运算的具体含义. 前置: https://leetcode-cn.com/tag/bit-manipulation/problemset/
		class IntSet
		{
		private:
			List<int> buffer;
		public:
			IntSet()
			{}
			IntSet(const IntSet & other)
			{
				buffer = other.buffer;
			}
			IntSet(IntSet && other)
			{
				*this = (_Move(other));
			}
			IntSet & operator = (IntSet && other)
			{
				buffer = _Move(other.buffer);
				return *this;
			}
			IntSet & operator = (const IntSet & other)
			{
				buffer = other.buffer;
				return *this;
			}
			IntSet(int maxVal)
			{
				SetMax(maxVal);
			}

			int Size() const
			{
				return buffer.Count()*32;
			}
			void SetMax(int val)
			{
				Resize(val);
				Clear();
			}
			void Resize(int size)
			{
				int oldBufferSize = buffer.Count();
				buffer.SetSize((size+31)>>5);
				if (buffer.Count() > oldBufferSize)
					memset(buffer.Buffer()+oldBufferSize, 0, (buffer.Count()-oldBufferSize) * sizeof(int));
			}
			void Clear()
			{
				memset(buffer.Buffer(), 0, sizeof(int)*buffer.Count());
			}
			void Add(int val)
			{
				int id = val>>5;	// id = var/32  这里不是取余，所以得到的是32的整数倍，意义是?
				if (id < buffer.Count())
					buffer[id] |= (1<<(val&31));	//优先级  & 大于 <<，这里对&31其实就是在取余，然后左移对应的位数，就可以变成一个2^n的形式，进行或运算，就是为了把对应的位设置成true.? 待。
				else
				{
					int oldSize = buffer.Count();
					buffer.SetSize(id+1);
					memset(buffer.Buffer() + oldSize, 0, (buffer.Count()-oldSize)*sizeof(int));
					buffer[id] |= (1<<(val&31));
				}
			}
			void Remove(int val)
			{
				if ((val>>5) < buffer.Count())
					buffer[(val>>5)] &= ~(1<<(val&31));
			}
			bool Contains(int val) const
			{
				if ((val>>5) >= buffer.Count())
					return false;
				return (buffer[(val>>5)] & (1<<(val&31))) != 0;	// https://blog.csdn.net/actionzh/article/details/78976082 位运算用来取余
			}
			void UnionWith(const IntSet & set)
			{
				for (int i = 0; i<Math::Min(set.buffer.Count(), buffer.Count()); i++)
				{
					buffer[i] |= set.buffer[i];
				}
				if (set.buffer.Count() > buffer.Count())
					buffer.AddRange(set.buffer.Buffer()+buffer.Count(), set.buffer.Count()-buffer.Count());
			}
			bool operator == (const IntSet & set)
			{
				if (buffer.Count() != set.buffer.Count())
					return false;
				for (int i = 0; i<buffer.Count(); i++)
					if (buffer[i] != set.buffer[i])
						return false;
				return true;
			}
			bool operator != (const IntSet & set)
			{
				return !(*this == set);
			}
			void IntersectWith(const IntSet & set)
			{
				if (set.buffer.Count() < buffer.Count())
					memset(buffer.Buffer() + set.buffer.Count(), 0, (buffer.Count()-set.buffer.Count())*sizeof(int));
				for (int i = 0; i<Math::Min(set.buffer.Count(), buffer.Count()); i++)
				{
					buffer[i] &= set.buffer[i];
				}
			}
			static void Union(IntSet & rs, const IntSet & set1, const IntSet & set2)
			{
				rs.buffer.SetSize(Math::Max(set1.buffer.Count(), set2.buffer.Count()));
				rs.Clear();
				for (int i = 0; i<set1.buffer.Count(); i++)
					rs.buffer[i] |= set1.buffer[i];
				for (int i = 0; i<set2.buffer.Count(); i++)
					rs.buffer[i] |= set2.buffer[i];
			}
			static void Intersect(IntSet & rs, const IntSet & set1, const IntSet & set2)
			{
				rs.buffer.SetSize(Math::Min(set1.buffer.Count(), set2.buffer.Count()));
				for (int i = 0; i<rs.buffer.Count(); i++)
					rs.buffer[i] = set1.buffer[i] & set2.buffer[i];
			}
			
		
			//java 里的说法是 ，
			//Return a new IntSet object containing all elements that are present in the current set but not present in the input set a.
			//https://doc.qt.io/archives/qt-4.8/qset.html#subtract Qt里也有类似的。 先看看其源码是否如此吧！
			// https://github.com/qt/qtbase/blob/9db7cc79a26ced4997277b5c206ca15949133240/src/corelib/tools/qset.h#L305 人家直接hash计算的..

			static void Subtract(IntSet & rs, const IntSet & set1, const IntSet & set2)
			{
				rs.buffer.SetSize(set1.buffer.Count());
				for (int i = 0; i < Math::Min(set1.buffer.Count(), set2.buffer.Count()); i++){
					rs.buffer[i] = set1.buffer[i] & (~set2.buffer[i]);
				}//抹除 set1.buffer[i] 中含有 set2.buffer[i] 的标志位？
			}/*
			你怎么知道就“应该”
				“应该”的话remove和contain会写成那样吗
				很明显buffer[0]的32个位表示了0 - 31是否存在
				第二个数字就是32 - 63
				但是不知道这样做好在哪，因为他要求数字是连续的。你突然插入一个很大的数字就得在中间加入一大串0*/
			static bool HasIntersection(const IntSet & set1, const IntSet & set2)
			{
				for (int i = 0; i<Math::Min(set1.buffer.Count(), set2.buffer.Count()); i++)
				{
					if (set1.buffer[i] & set2.buffer[i])
						return true;
				}
				return false;
			}
		};
	}
}

#endif