#ifndef FUNDAMENTAL_LIB_LIST_H
#define FUNDAMENTAL_LIB_LIST_H
#include <type_traits>

const int MIN_QSORT_SIZE = 32;	
//猜测一下是小于32时候不执行快排？
//数据量小就插入，大才快排,就是这样。

namespace CoreLib
{
	namespace Basic
	{
		template<typename T>
		class List	//like vector
		{
		private:
			static const int InitialSize = 16;
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
				: buffer(0), _count(0), bufferSize(0)
			{
			}
			List(const List<T> & list)
				: buffer(0), _count(0), bufferSize(0)
			{
				this->operator=(list);
			}
			List(List<T> && list)
				: buffer(0), _count(0), bufferSize(0)
			{
				//int t = static_cast<int>(1.0f); reinterpret_cast<double*>(&t), dynamic_cast<> 
				this->operator=(static_cast<List<T>&&>(list));
			}
			~List()
			{
				Free();
			}
			List<T> & operator=(const List<T> & list)
			{
				Free();
				AddRange(list);

				return *this;
			}

			List<T> & operator=(List<T> && list)
			{
				Free();
				_count = list._count;
				bufferSize = list.bufferSize;
				buffer = list.buffer;

				list.buffer = 0;
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
				buffer[_count++] = static_cast<T&&>(obj);
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

					T * newBuffer = new T[newBufferSize];
					if (bufferSize)
					{
						//if (std::has_trivial_assign<T>::value ||
						//	std::has_trivial_copy<T>::value)
						if (std::is_trivially_copy_assignable<T>::value ||
							std::is_trivially_copy_constructible<T>::value)
						{
							memcpy(newBuffer, buffer, sizeof(T) * id);
							memcpy(newBuffer + id + n, buffer + id, sizeof(T) * (_count - id));
						}
						else
						{
							for (int i = 0; i < id; i++)
								newBuffer[i] = buffer[i];
							for (int i = id; i < _count; i++)
								newBuffer[i + n] = T(static_cast<T&&>(buffer[i]));
						}
						delete [] buffer;
					}
					buffer = newBuffer;
					bufferSize = newBufferSize;
				}
				else
				{
					if (std::is_trivially_copy_assignable<T>::value ||
						std::is_trivially_copy_constructible<T>::value)
						memmove(buffer + id + n, buffer + id, sizeof(T) * (_count - id));
					else
					{
						for (int i = _count - 1; i >= id; i--)
							buffer[i + n] = static_cast<T&&>(buffer[i]);
					}
				}
				if (std::is_trivially_copy_assignable<T>::value ||
					std::is_trivially_copy_constructible<T>::value)
					memcpy(buffer + id, vals, sizeof(T) * n);
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
				int actualDeleteCount = ((id + deleteCount) >= _count)? (_count - id) : deleteCount;
				for (int i = id + actualDeleteCount; i < _count; i++)
					buffer[i - actualDeleteCount] = static_cast<T&&>(buffer[i]);
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
				if (idx != -1 && _count-1 != idx)
				{
					buffer[idx] = _Move(buffer[_count-1]);
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
					T * newBuffer = new T[size];
					if (bufferSize)
					{
						if (std::is_trivially_copy_assignable<T>::value ||std::is_trivially_copy_constructible<T>::value)
							memcpy(newBuffer, buffer, _count * sizeof(T));
						else
						{
							for (int i = 0; i < _count; i++)
								newBuffer[i] = static_cast<T&&>(buffer[i]);
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
					T * newBuffer = new T[_count];
					for (int i = 0; i < _count; i++)
						newBuffer[i] = static_cast<T&&>(buffer[i]);
					delete [] buffer;
					buffer = newBuffer;
					bufferSize = _count;
				}
			}

			T & operator [](int id) const
			{
#if _DEBUG
				if(id >= _count || id < 0)
					throw IndexOutofRangeException(L"Operator[]: Index out of Range.");
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

			bool Contains(const T & val)
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
					T insertValue = static_cast<T&&>(vals[i]);
					int insertIndex = i - 1;
					while (insertIndex >= startIndex && comparer(insertValue, vals[insertIndex]))
					{
						vals[insertIndex + 1] = static_cast<T&&>(vals[insertIndex]);
						insertIndex--;
					}
					vals[insertIndex + 1] = static_cast<T&&>(insertValue);
				}
			}

			inline void Swap(T * vals, int index1, int index2)
			{
				if (index1 != index2)
				{
					T tmp = static_cast<T&&>(vals[index1]);
					vals[index1] = static_cast<T&&>(vals[index2]);
					vals[index2] = static_cast<T&&>(tmp);
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