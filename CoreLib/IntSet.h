#ifndef BIT_VECTOR_INT_SET_H
#define BIT_VECTOR_INT_SET_H

#include <memory.h>
#include<iostream>
#include "List.h"
#include "LibMath.h"
#include "Common.h"
#include "Exception.h"
namespace CoreLib
{
	namespace Basic
	{
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
			IntSet(IntSet && other)noexcept
			{
				*this = (_Move(other));
			}
			IntSet & operator = (IntSet && other)noexcept
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

			//2^5 ==32,buffer*32,每一个数字占其中某一个二进制位，int个数*32就是能装得下的所有数字。
			int Size() const
			{
				return buffer.Count()*32;	
			}

			void SetMax(int val)
			{
				Resize(val);
				Clear();
			}
			//调整当前容器里int的个数所代表的所有位可以存的下size个不同的值。
			void Resize(int size)
			{
				int oldBufferSize = buffer.Count();
				buffer.SetSize((size+31)>>5);	
				if (buffer.Count() > oldBufferSize)	//把新加的塞进去.
					memset(buffer.Buffer()+oldBufferSize, 0, (buffer.Count()-oldBufferSize) * sizeof(int));
			}
			void Clear()
			{
				memset(buffer.Buffer(), 0, sizeof(int)*buffer.Count());
			}
			void Add(int val)
			{
				int id = val>>5;	// id = var/32  这里不是取余，所以得到的是32的整数倍，意义是 找到list中的第i个索引
				if (id < buffer.Count())
					buffer[id] |= (1<<(val&31));	
				//优先级  & 大于 <<，这里对&31其实就是在取余，只要最低五位.
				//然后左移val对应的位数，将该位置的值设置为1。
				//接着进行或运算，把对应的位设置成true。
				else//否则就得先调节然后再设置。
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
				}//抹除 set1.buffer[i] 中含有 set2.buffer[i] 的标志位。
			}
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
