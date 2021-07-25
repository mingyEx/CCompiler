#ifndef SCOPE_DICTIONARY_H
#define SCOPE_DICTIONARY_H

namespace Compiler
{
	namespace Intermediate
	{
		template <typename TKey, typename TValue>
		class ScopeDictionary
		{
		public:
			LinkedList<Dictionary<TKey, TValue>> dicts;
		public:
			void PushScope()
			{
				dicts.AddLast();
			}
			void PopScope()
			{
				dicts.Delete(dicts.LastNode());
			}
			bool TryGetValue(const TKey & key, TValue & value)
			{
				for (auto iter = dicts.LastNode(); iter; iter = iter->GetPrevious())	//一点点往前找遍每个字典，并且递归调用.
				{
					bool rs = iter->Value.TryGetValue(key, value);
				//value的类型是 :CoreLib::Basic::Dictionary<CoreLib::Basic::String,Compiler::Intermediate::Operand>,所以也是以dict来调用，
				//		那么... 算了，明明就只是在递归调用，每次都摸到dicts里具体某个值而已..
					//为什么调试的时候，调用里面的时候不会递归呢? 好吧，虽然名字参数都一样，但是调用对象分别似乎dict和dicts. 
					//总之，这个是在，遍历所有的，只要有一个成立就return true. value带返回值吗？是的.

					if (rs)
						return true;
				}
				return false;
			}
			bool TryGetValueInCurrentScope(const TKey & key, TValue & value)
			{
				return dicts.Last().TryGetValue(key, value);
			}
			void Add(const TKey & key, const TValue & value)
			{
				dicts.Last().Add(key, value);
			}
		};
	}
}

#endif