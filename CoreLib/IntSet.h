#ifndef BIT_VECTOR_INT_SET_H
#define BIT_VECTOR_INT_SET_H

#include <algorithm>
#include <cstring>
#include <vector>

namespace CoreLib
{
	namespace Basic
	{
		class IntSet
		{
		private:	
			std::vector<int> buffer;
		public:
			IntSet() = default;
			IntSet(const IntSet & other) = default;
			IntSet(IntSet && other) noexcept = default;
			IntSet & operator = (IntSet && other) noexcept = default;
			IntSet & operator = (const IntSet & other) = default;
			IntSet(int maxVal)
			{
				SetMax(maxVal);
			}

			//2^5 ==32,buffer*32,每一个数字占其中某一个二进制位，int个数*32就是能装得下的所有数字。
			int Size() const
			{
				return static_cast<int>(buffer.size()) * 32;
			}

			void SetMax(int val)
			{
				Resize(val);
				Clear();
			}
			//调整当前容器里int的个数所代表的所有位可以存的下size个不同的值。
			void Resize(int size)
			{
				const int old_buffer_size = static_cast<int>(buffer.size());
				buffer.resize(static_cast<size_t>((size + 31) >> 5));
				if (static_cast<int>(buffer.size()) > old_buffer_size)
					std::memset(buffer.data() + old_buffer_size, 0, (buffer.size() - static_cast<size_t>(old_buffer_size)) * sizeof(int));
			}
			void Clear()
			{
				if (!buffer.empty())
					std::memset(buffer.data(), 0, sizeof(int) * buffer.size());
			}
			void Add(int val)
			{
				const size_t id = static_cast<size_t>(val >> 5);
				if (id < buffer.size())
					buffer[id] |= (1 << (val & 31));
				//优先级  & 大于 <<，这里对&31其实就是在取余，只要最低五位.
				//然后左移val对应的位数，将该位置的值设置为1。
				//接着进行或运算，把对应的位设置成true。
				else//否则就得先调节然后再设置。
				{
					const size_t old_size = buffer.size();
					buffer.resize(id + 1);
					std::memset(buffer.data() + old_size, 0, (buffer.size() - old_size) * sizeof(int));
					buffer[id] |= (1 << (val & 31));
				}
			}
			void Remove(int val)
			{
				const size_t id = static_cast<size_t>(val >> 5);
				if (id < buffer.size())
					buffer[id] &= ~(1 << (val & 31));
			}
			bool Contains(int val) const
			{
				const size_t id = static_cast<size_t>(val >> 5);
				if (id >= buffer.size())
					return false;
				return (buffer[id] & (1 << (val & 31))) != 0;
			}
			void UnionWith(const IntSet & set)
			{
				for (size_t i = 0; i < (std::min)(set.buffer.size(), buffer.size()); i++)
				{
					buffer[i] |= set.buffer[i];
				}
				if (set.buffer.size() > buffer.size())
					buffer.insert(
						buffer.end(),
						set.buffer.begin() + static_cast<std::ptrdiff_t>(buffer.size()),
						set.buffer.end());
			}
			bool operator == (const IntSet & set)
			{
				if (buffer.size() != set.buffer.size())
					return false;
				for (size_t i = 0; i < buffer.size(); i++)
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
				if (set.buffer.size() < buffer.size())
					std::memset(buffer.data() + set.buffer.size(), 0, (buffer.size() - set.buffer.size()) * sizeof(int));
				for (size_t i = 0; i < (std::min)(set.buffer.size(), buffer.size()); i++)
				{
					buffer[i] &= set.buffer[i];
				}
			}
			static void Union(IntSet & rs, const IntSet & set1, const IntSet & set2)
			{
				rs.buffer.resize((std::max)(set1.buffer.size(), set2.buffer.size()));
				rs.Clear();
				for (size_t i = 0; i < set1.buffer.size(); i++)
					rs.buffer[i] |= set1.buffer[i];
				for (size_t i = 0; i < set2.buffer.size(); i++)
					rs.buffer[i] |= set2.buffer[i];
			}
			static void Intersect(IntSet & rs, const IntSet & set1, const IntSet & set2)
			{
				rs.buffer.resize((std::min)(set1.buffer.size(), set2.buffer.size()));
				for (size_t i = 0; i < rs.buffer.size(); i++)
					rs.buffer[i] = set1.buffer[i] & set2.buffer[i];
			}
			
		
			static void Subtract(IntSet & rs, const IntSet & set1, const IntSet & set2)
			{
				rs.buffer.resize(set1.buffer.size());
				for (size_t i = 0; i < (std::min)(set1.buffer.size(), set2.buffer.size()); i++) {
					rs.buffer[i] = set1.buffer[i] & (~set2.buffer[i]);
				}//抹除 set1.buffer[i] 中含有 set2.buffer[i] 的标志位。
			}
			static bool HasIntersection(const IntSet & set1, const IntSet & set2)
			{
				for (size_t i = 0; i < (std::min)(set1.buffer.size(), set2.buffer.size()); i++)
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
