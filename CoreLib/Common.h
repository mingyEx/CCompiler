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
		inline T&& _Move(T & obj)
		{
			return static_cast<T&&>(obj);
		}
		//只能传递左值
		// std::move的定义是这样的,可以接受任意类型
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