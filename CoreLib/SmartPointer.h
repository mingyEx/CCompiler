#ifndef FUNDAMENTAL_LIB_SMART_POINTER_H
#define FUNDAMENTAL_LIB_SMART_POINTER_H

#include <utility>

namespace CoreLib
{
	namespace Basic
	{
		//引用指针的默认析构？
		class RefPtrDefaultDestructor	
		{
		public:
			template<typename T>
			void operator ()(T * ptr)
			{
				delete ptr;
			}
		};

		class RefPtrArrayDestructor
		{
		public:
			template<typename T>
			void operator() (T * ptr)
			{
				delete [] ptr;
			}
		};

		template<typename T, typename Destructor = RefPtrDefaultDestructor>
		class RefPtr
		{
			template<typename T1, typename Destructor1>
			friend class RefPtr;	//不同类型作为参数的自己是友元，用来在ctor里摸对象。
		private:
			T *pointer;
			int *refCount;
			
		public:
			RefPtr():pointer(nullptr), refCount(nullptr){}
			RefPtr(T * ptr): pointer(nullptr), refCount(nullptr)
			{
				this->operator=(ptr);
			}
			template<typename T1>
			RefPtr(T1 * ptr)	
				//泛型构造函数，ptr会被推断给operator=用。
				: pointer(nullptr), refCount(nullptr)
			{
				this->operator=(ptr);
			}
			RefPtr(const RefPtr<T, Destructor> & ptr)
				: pointer(nullptr), refCount(nullptr)
			{
				this->operator=(ptr);
			}
			RefPtr(RefPtr<T, Destructor> && str)
				: pointer(nullptr), refCount(nullptr)
			{
				this->operator=(std::move(str));
			}
			
			RefPtr<T,Destructor>& operator=(T * ptr)
			{
				if (pointer == ptr)
					return *this;
				Dereferance();
				pointer = ptr;
				if(ptr)
				{
					refCount = new int(1);
				}
				else
					refCount = nullptr;
				return *this;
			}
			template<typename T1>
			RefPtr<T,Destructor>& operator=(T1 * ptr)
			{
				if (static_cast<const void*>(pointer) == static_cast<const void*>(ptr))
					return *this;
				Dereferance();
				pointer = dynamic_cast<T*>(ptr);	//为啥这里反倒dym了..
				if(pointer)
				{
					refCount = new int(1);
				}
				else
					refCount = nullptr;
				return *this;
			}
			RefPtr<T,Destructor>& operator=(const RefPtr<T, Destructor> & ptr)
			{
				if(ptr.pointer != pointer)
				{
					Dereferance();
					pointer = ptr.pointer;
					refCount = ptr.refCount;
					if (refCount)
						(*refCount)++;
				}
				return *this;
			}

			template<typename T1>
			RefPtr(const RefPtr<T1> & ptr)
				: pointer(nullptr), refCount(nullptr)
			{
				this->operator=(ptr);
			}

			template<typename T1>
			RefPtr<T,Destructor> & operator = (const RefPtr<T1, Destructor> & ptr)
			{
				if(static_cast<const void*>(ptr.pointer) != static_cast<const void*>(pointer))
				{
					Dereferance();
					pointer = dynamic_cast<T*>(ptr.pointer);
					if (pointer && ptr.refCount)
					{
						refCount = ptr.refCount;
						(*refCount)++;
					}
					else
					{
						pointer = nullptr;
						refCount = nullptr;
					}
				}
				return *this;
			}
			
			bool operator == (const T * ptr) const
			{
				return pointer == ptr;
			}
			bool operator != (const T * ptr) const
			{
				return pointer != ptr;
			}
			bool operator == (const RefPtr<T> & ptr) const
			{
				return pointer == ptr.pointer;
			}
			bool operator != (const RefPtr<T> & ptr) const
			{
				return pointer != ptr.pointer;
			}

			T* operator +(int offset) const
			{
				return pointer+offset;
			}
			T& operator [](int idx) const
			{
				return *(pointer + idx);	
				//实现了[]操作符，让用指针指向的数组内的元素可以以[]来访问.
			}
			RefPtr<T,Destructor>& operator=(RefPtr<T, Destructor> && ptr)
			{
				if(ptr.pointer != pointer)
				{
					Dereferance();
					pointer = ptr.pointer;
					refCount = ptr.refCount;
					ptr.pointer = nullptr;
					ptr.refCount = nullptr;
				}
				return *this;
			}
			T* Release()
			{
				if(pointer)
				{
					if((*refCount) > 1)
					{
						(*refCount)--;
					}
					else
					{
						delete refCount;
					}
				}
				auto rs = pointer;
				refCount = nullptr;
				pointer = nullptr;
				return rs;
			}
			~RefPtr()
			{
				Dereferance();
			}

			void Dereferance()
			{
				if(pointer)
				{
					if((*refCount) > 1)
					{
						(*refCount)--;
					}
					else
					{
						Destructor destructor;	//析构还是自备.
						destructor(pointer);
						delete refCount;
					}
					pointer = nullptr;
					refCount = nullptr;
				}
			}
			T & operator *() const
			{
				return *pointer;
			}
			T * operator->() const
			{
				return pointer;
			}
			T * Ptr() const
			{
				return pointer;
			}
		public:
			operator void*() const 
			{
				return static_cast<void*>(pointer);
			}
		};
	}
}

#endif
