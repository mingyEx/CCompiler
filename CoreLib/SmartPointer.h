#ifndef FUNDAMENTAL_LIB_SMART_POINTER_H
#define FUNDAMENTAL_LIB_SMART_POINTER_H

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
				: pointer(0), refCount(0)
			{
				this->operator=(ptr);
			}
			RefPtr(const RefPtr<T, Destructor> & ptr)
				: pointer(0), refCount(0)
			{
				this->operator=(ptr);
			}
			RefPtr(RefPtr<T, Destructor> && str)
				: pointer(0), refCount(0)
			{
				this->operator=(static_cast<RefPtr<T, Destructor> &&>(str));	
				//// static_cast 没有运行时类型检查来保证转换的安全性,
				// lyt的这个是dynamic版本的，有啥区别?
				//如果 新类型 是指向某类类型 D 的指针或引用，且 表达式 的类型是其非虚基类 B 的左值或指向它的指针纯右值,
				//则 static_cast 进行向下转型（downcast）
				//	这种 static_cast 并不在运行时检查该对象的运行时类型确实为 D, 安全的向下转型可以用 dynamic_cast 执行
			}
			
			RefPtr<T,Destructor>& operator=(T * ptr)
			{
				Dereferance();
				pointer = ptr;
				if(ptr)
				{
					refCount = new int;		
					(*refCount) = 1;
				}
				else
					refCount = 0;
				return *this;
			}
			template<typename T1>
			RefPtr<T,Destructor>& operator=(T1 * ptr)
			{
				Dereferance();
				pointer = dynamic_cast<T*>(ptr);	//为啥这里反倒dym了..
				if(ptr)
				{
					refCount = new int;
					(*refCount) = 1;
				}
				else
					refCount = 0;
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
				: pointer(0), refCount(0)
			{
				this->operator=(ptr);
			}

			template<typename T1>
			RefPtr<T,Destructor> & operator = (const RefPtr<T1, Destructor> & ptr)
			{
				if(ptr.pointer != pointer)
				{
					Dereferance();
					pointer = dynamic_cast<T*>(ptr.pointer);
					refCount = ptr.refCount;
					(*refCount)++;
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
					ptr.pointer = 0;
					ptr.refCount = 0;
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
				refCount = 0;
				pointer = 0;
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
		private:
			class _BoolConversionClass	//没用..
			{
			public:
				int Dummy;
			};
		public:
			operator void*() const 
			{
				if (pointer)
					return (void*)(pointer);
				else
					return 0;
			}
		};
	}
}

#endif