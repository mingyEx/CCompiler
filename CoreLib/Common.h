#ifndef CORE_LIB_COMMON_H
#define CORE_LIB_COMMON_H
#include<type_traits>
namespace CoreLib
{
	namespace Basic
	{
		class Object
		{
		public:
			virtual ~Object(){}
		};
		template <typename T>
		inline T&& _Move(T & obj)		//直接用std::move有什么问题吗？ 没找到它的实现. 如果自己写，试一下。
		{
			return static_cast<T&&>(obj);
		}

		// std::move的定义是这样的		 有什么区别啊? 这里的可以接受任意类型，为什么要remove呢？ 为了可以传递给他左值或者右值，而上面的似乎只能传递左值. 吗.
		//template<typename T>
		//typename std::remove_reference<T>::type&& move(T&& t)
		//{
		//	return static_cast<typename std::remove_reference<T>::type&&>(t);
		//}
		inline int FloatAsInt(float val)
		{
			return *(int*)&val;				//运算符优先级相同，只是转换类型。
											//只能想到一点，转的是指针，所以不丢失精度。
		}
	}
}

#endif