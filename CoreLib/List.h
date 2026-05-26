#ifndef FUNDAMENTAL_LIB_LIST_H
#define FUNDAMENTAL_LIB_LIST_H
#include <cstring>
#include <type_traits>
#include <utility>
#include "Exception.h"

const int MIN_QSORT_SIZE = 32;	
//�²�һ����С��32ʱ��ִ�п��ţ�
//������С�Ͳ��룬��ſ���,����������

namespace CoreLib
{
	namespace Basic
	{
		template<typename T>
		class List	//like vector
		{
		private:
			static const int InitialSize = 16;
			static constexpr bool CanCopyWithMemcpy =
				std::is_trivially_copyable_v<T>;
		private:
			T * buffer;
			int _count;
			int bufferSize;

			void Free()
			{
				if (buffer)
					delete [] buffer;
				buffer = nullptr;
				_count = bufferSize = 0;
			}
		public:
			T* begin() const
			{
				return buffer;
			}
			T* end() const
			{
				return buffer+_count;
			}
		public:
			List()
				: buffer(nullptr), _count(0), bufferSize(0)
			{
			}
			List(const List<T> & list)
				: buffer(nullptr), _count(0), bufferSize(0)
			{
				this->operator=(list);
			}
			List(List<T> && list)
				: buffer(nullptr), _count(0), bufferSize(0)
			{
				this->operator=(std::move(list));
			}
			~List()
			{
				Free();
			}
			List<T> & operator=(const List<T> & list)
			{
				if (this == &list)
					return *this;
				Free();
				AddRange(list);

				return *this;
			}

			List<T> & operator=(List<T> && list)
			{
				if (this == &list)
					return *this;
				Free();
				_count = list._count;
				bufferSize = list.bufferSize;
				buffer = list.buffer;

				list.buffer = nullptr;
				list._count = 0;
				list.bufferSize = 0;
				return *this;
			}

			T & First() const
			{
#ifdef _DEBUG
				if (_count == 0)
					throw "Index out of range.";
#endif
				return buffer[0];
			}

			T & Last() const
			{
#ifdef _DEBUG
				if (_count == 0)
					throw "Index out of range.";
#endif
				return buffer[_count-1];
			}

			void Add(T && obj)
			{
				if (bufferSize < _count + 1)
				{
					int newBufferSize = InitialSize;
					if (bufferSize)
						newBufferSize = (bufferSize << 1);

					Reserve(newBufferSize);
				}
				buffer[_count++] = std::move(obj);
			}

			void Add(const T & obj)
			{
				if (bufferSize < _count + 1)
				{
					int newBufferSize = InitialSize;
					if (bufferSize)
						newBufferSize = (bufferSize << 1);

					Reserve(newBufferSize);
				}
				buffer[_count++] = obj;

			}

			int Count() const
			{
				return _count;
			}

			T* Buffer() const
			{
				return buffer;
			}

			int Capacity() const
			{
				return bufferSize;
			}

			void Insert(int id, const T & val)
			{
				InsertRange(id, &val, 1);
			}

			void InsertRange(int id, const T * vals, int n)
			{
				if (bufferSize < _count + n)
				{
					int newBufferSize = InitialSize;
					while (newBufferSize < _count + n)
						newBufferSize = newBufferSize << 1;

					T * newBuffer = new T[static_cast<size_t>(newBufferSize)];
					if (bufferSize)
					{
						if constexpr (CanCopyWithMemcpy)
						{
							std::memcpy(newBuffer, buffer, sizeof(T) * static_cast<size_t>(id));
							std::memcpy(newBuffer + id + n, buffer + id, sizeof(T) * static_cast<size_t>(_count - id));
						}
						else
						{
							for (int i = 0; i < id; i++)
								newBuffer[i] = buffer[i];
							for (int i = id; i < _count; i++)
								newBuffer[i + n] = T(std::move(buffer[i]));
						}
						delete [] buffer;
					}
					buffer = newBuffer;
					bufferSize = newBufferSize;
				}
				else
				{
					if constexpr (CanCopyWithMemcpy)
						std::memmove(buffer + id + n, buffer + id, sizeof(T) * static_cast<size_t>(_count - id));
					else
					{
						for (int i = _count - 1; i >= id; i--)
							buffer[i + n] = std::move(buffer[i]);
					}
				}
				if constexpr (CanCopyWithMemcpy)
					std::memcpy(buffer + id, vals, sizeof(T) * static_cast<size_t>(n));
				else
					for (int i = 0; i < n; i++)
						buffer[id + i] = vals[i];

				_count += n;
			}			
			
			void InsertRange(int id, const List<T> & list)
			{
				InsertRange(id, list.buffer, list._count);
			}

			void AddRange(const T * vals, int n)
			{
				InsertRange(_count, vals, n);
			}

			void AddRange(const List<T> & list)
			{
				InsertRange(_count, list.buffer, list._count);
			}

			void RemoveRange(int id, int deleteCount)
			{
#if _DEBUG
				if (id >= _count || id < 0)
					throw "Remove: Index out of range.";
				if(deleteCount < 0)
					throw "Remove: deleteCount smaller than zero.";
#endif
				if (id < 0 || id >= _count || deleteCount <= 0)
					return;
				int actualDeleteCount = ((id + deleteCount) >= _count)? (_count - id) : deleteCount;
				for (int i = id + actualDeleteCount; i < _count; i++)
					buffer[i - actualDeleteCount] = std::move(buffer[i]);
				_count -= actualDeleteCount;
			}

			void RemoveAt(int id)
			{
				RemoveRange(id, 1);
			}

			void Remove(const T & val)
			{
				int idx = IndexOf(val);
				if (idx != -1)
					RemoveAt(idx);
			}

			void FastRemove(const T & val)
			{
				int idx = IndexOf(val);
				if (idx == -1)
					return;
				if (_count-1 != idx)
				{
					buffer[idx] = std::move(buffer[_count-1]);
				}
				_count--;
			}

			void Clear()
			{
				_count = 0;
			}

			void Reserve(int size)
			{
				if(size > bufferSize)
				{
					T * newBuffer = new T[static_cast<size_t>(size)];
					if (bufferSize)
					{
						if constexpr (CanCopyWithMemcpy)
							std::memcpy(newBuffer, buffer, sizeof(T) * static_cast<size_t>(_count));
						else
						{
							for (int i = 0; i < _count; i++)
								newBuffer[i] = std::move(buffer[i]);
						}
						delete [] buffer;
					}
					buffer = newBuffer;
					bufferSize = size;
				}
			}

			void SetSize(int size)
			{
				Reserve(size);
				_count = size;
			}

			void Compress()
			{
				if (bufferSize > _count && _count > 0)
				{
					T * newBuffer = new T[static_cast<size_t>(_count)];
					for (int i = 0; i < _count; i++)
						newBuffer[i] = std::move(buffer[i]);
					delete [] buffer;
					buffer = newBuffer;
					bufferSize = _count;
				}
			}

			T & operator [](int id) const
			{
#if _DEBUG
				if(id >= _count || id < 0)
					throw CoreLib::Basic::IndexOutofRangeException(L"Operator[]: Index out of Range.");
#endif
				return buffer[id];
			}

			template<typename T2>
			int IndexOf(const T2 & val) const
			{
				for (int i = 0; i < _count; i++)
				{
					if (buffer[i] == val)
						return i;
				}
				return -1;
			}

			template<typename T2>
			int LastIndexOf(const T2 & val) const
			{
				for (int i = _count - 1; i >= 0; i--)
				{
					if(buffer[i] == val)
						return i;
				}
				return -1;
			}

			void Sort()
			{
				Sort([](T& t1, T& t2){return t1<t2;});
			}

			bool Contains(const T & val) const
			{
				for (int i = 0; i<_count; i++)
					if (buffer[i] == val)
						return true;
				return false;
			}

			template<typename Comparer>
			void Sort(Comparer compare)
			{
				//InsertionSort(buffer, 0, _count - 1);
				QuickSort(buffer, 0, _count - 1, compare);
			}

			template <typename IterateFunc>
			void ForEach(IterateFunc f) const
			{
				for (int i = 0; i<_count; i++)
					f(buffer[i]);
			}

			template<typename Comparer>
			void QuickSort(T * vals, int startIndex, int endIndex, Comparer comparer)
			{
				if(startIndex < endIndex)
				{
					if (endIndex - startIndex < MIN_QSORT_SIZE)
						InsertionSort(vals, startIndex, endIndex, comparer);
					else
					{
						int pivotIndex = (startIndex + endIndex) >> 1;
						int pivotNewIndex = Partition(vals, startIndex, endIndex, pivotIndex, comparer);
						QuickSort(vals, startIndex, pivotNewIndex - 1, comparer);
						QuickSort(vals, pivotNewIndex + 1, endIndex, comparer);
					}
				}

			}
			template<typename Comparer>
			int Partition(T * vals, int left, int right, int pivotIndex, Comparer comparer)
			{
				T pivotValue = vals[pivotIndex];
				Swap(vals, right, pivotIndex);
				int storeIndex = left;
				for (int i = left; i < right; i++)
				{
					if (comparer(vals[i], pivotValue))
					{
						Swap(vals, i, storeIndex);
						storeIndex++;
					}
				}
				Swap(vals, storeIndex, right);
				return storeIndex;
			}
			template<typename Comparer>
			void InsertionSort(T * vals, int startIndex, int endIndex, Comparer comparer)
			{
				for (int i = startIndex  + 1; i <= endIndex; i++)
				{
					T insertValue = std::move(vals[i]);
					int insertIndex = i - 1;
					while (insertIndex >= startIndex && comparer(insertValue, vals[insertIndex]))
					{
						vals[insertIndex + 1] = std::move(vals[insertIndex]);
						insertIndex--;
					}
					vals[insertIndex + 1] = std::move(insertValue);
				}
			}

			inline void Swap(T * vals, int index1, int index2)
			{
				if (index1 != index2)
				{
					T tmp = std::move(vals[index1]);
					vals[index1] = std::move(vals[index2]);
					vals[index2] = std::move(tmp);
				}
			}

			template<typename T2, typename Comparer>
			int BinarySearch(const T2 & obj, Comparer comparer)
			{
				int imin = 0, imax = _count - 1;
				while (imax >= imin)
				{
					int imid = (imin + imax) >> 1;
					int compareResult = comparer(buffer[imid], obj);
					if (compareResult == 0)
						return imid;
					else if (compareResult < 0)
						imin = imid + 1;
					else
						imax = imid - 1;
				}
				return -1;
			}

			template<typename T2>
			int BinarySearch(const T2 & obj)
			{
				return BinarySearch(obj, 
					[](T & curObj, const T2 & thatObj)->int
					{
						if (curObj < thatObj)
							return -1;
						else if (curObj == thatObj)
							return 0;
						else
							return 1;
					});
			}
		};

		template<typename T>
		T Min(const List<T> & list)
		{
			T minVal = list.First();
			for (int i = 1; i<list.Count(); i++)
				if (list[i] < minVal)
					minVal = list[i];
			return minVal;
		}

		template<typename T>
		T Max(const List<T> & list)
		{
			T maxVal = list.First();
			for (int i = 1; i<list.Count(); i++)
				if (list[i] > maxVal)
					maxVal = list[i];
			return maxVal;
		}
	}
}

#endif
