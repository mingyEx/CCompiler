#ifndef SCOPE_DICTIONARY_H
#define SCOPE_DICTIONARY_H

#include <unordered_map>
#include <vector>

namespace Compiler
{
	namespace Intermediate
	{
		template <typename TKey, typename TValue>
		class ScopeDictionary
		{
		public:
			using MapType = std::unordered_map<TKey, TValue>;
			std::vector<MapType> dicts;
		public:
			void PushScope()
			{
				dicts.emplace_back(MapType{});
			}
			void PopScope()
			{
				dicts.pop_back();
			}
			bool TryGetValue(const TKey & key, TValue & value)
			{
				for (auto iter = dicts.rbegin(); iter != dicts.rend(); ++iter)	//一点点往前找遍每个字典，并且递归调用.
				{
					auto value_iter = iter->find(key);
					bool rs = value_iter != iter->end();
				//value的类型是 :std::unordered_map<std::wstring, Compiler::Intermediate::Operand>，所以也是以dict来调用，
				//		那么... 算了，明明就只是在递归调用，每次都摸到dicts里具体某个值而已..
					//为什么调试的时候，调用里面的时候不会递归呢? 好吧，虽然名字参数都一样，但是调用对象分别似乎dict和dicts. 
					//总之，这个是在，遍历所有的，只要有一个成立就return true. value带返回值吗？是的.

					if (rs)
					{
						value = value_iter->second;
						return true;
					}
				}
				return false;
			}
			bool TryGetValueInCurrentScope(const TKey & key, TValue & value)
			{
				auto value_iter = dicts.back().find(key);
				if (value_iter == dicts.back().end())
					return false;
				value = value_iter->second;
				return true;
			}
			void Add(const TKey & key, const TValue & value)
			{
				dicts.back()[key] = value;
			}
		};
	}
}

#endif
