#ifndef CORE_LIB_COMMON_H
#define CORE_LIB_COMMON_H

#include <bit>
#include <utility>

namespace CoreLib
{
	namespace Basic
	{
		class Object
		{
		public:
			Object() = default;
			Object(const Object &) = default;
			Object & operator=(const Object &) = default;
			Object(Object &&) = default;
			Object & operator=(Object &&) = default;
			virtual ~Object(){}
		};

		inline int FloatAsInt(float val)
		{
			return std::bit_cast<int>(val);
		}
	}
}

#endif
