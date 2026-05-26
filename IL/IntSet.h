#ifndef COMPILER_INTERMEDIATE_INT_SET_H
#define COMPILER_INTERMEDIATE_INT_SET_H

#include <algorithm>
#include <cstddef>
#include <vector>

namespace Compiler
{
	namespace Intermediate
	{
		class IntSet
		{
		private:
			static constexpr int bits_per_word = 32;
			std::vector<unsigned int> words;

			static int WordIndex(int value)
			{
				return value / bits_per_word;
			}

			static unsigned int BitMask(int value)
			{
				return 1u << (value % bits_per_word);
			}

		public:
			IntSet() = default;

			explicit IntSet(int maxValue)
			{
				SetMax(maxValue);
			}

			int Size() const
			{
				return static_cast<int>(words.size()) * bits_per_word;
			}

			void SetMax(int value)
			{
				Resize(value);
				Clear();
			}

			void Resize(int size)
			{
				const auto wordCount = static_cast<std::size_t>(((std::max)(size, 0) + bits_per_word - 1) / bits_per_word);
				words.resize(wordCount, 0);
			}

			void Clear()
			{
				std::fill(words.begin(), words.end(), 0u);
			}

			void Add(int value)
			{
				if (value < 0)
					return;

				const auto index = static_cast<std::size_t>(WordIndex(value));
				if (index >= words.size())
					words.resize(index + 1, 0);
				words[index] |= BitMask(value);
			}

			void Remove(int value)
			{
				if (value < 0)
					return;

				const auto index = static_cast<std::size_t>(WordIndex(value));
				if (index < words.size())
					words[index] &= ~BitMask(value);
			}

			bool Contains(int value) const
			{
				if (value < 0)
					return false;

				const auto index = static_cast<std::size_t>(WordIndex(value));
				return index < words.size() && (words[index] & BitMask(value)) != 0;
			}

			void UnionWith(const IntSet& set)
			{
				if (set.words.size() > words.size())
					words.resize(set.words.size(), 0);
				for (std::size_t i = 0; i < set.words.size(); i++)
					words[i] |= set.words[i];
			}

			void IntersectWith(const IntSet& set)
			{
				const auto shared = (std::min)(words.size(), set.words.size());
				for (std::size_t i = 0; i < shared; i++)
					words[i] &= set.words[i];
				for (std::size_t i = shared; i < words.size(); i++)
					words[i] = 0;
			}

			bool operator == (const IntSet& set) const
			{
				const auto maxSize = (std::max)(words.size(), set.words.size());
				for (std::size_t i = 0; i < maxSize; i++)
				{
					const auto left = i < words.size() ? words[i] : 0u;
					const auto right = i < set.words.size() ? set.words[i] : 0u;
					if (left != right)
						return false;
				}
				return true;
			}

			bool operator != (const IntSet& set) const
			{
				return !(*this == set);
			}

			static void Union(IntSet& result, const IntSet& left, const IntSet& right)
			{
				result.words.assign((std::max)(left.words.size(), right.words.size()), 0);
				for (std::size_t i = 0; i < left.words.size(); i++)
					result.words[i] |= left.words[i];
				for (std::size_t i = 0; i < right.words.size(); i++)
					result.words[i] |= right.words[i];
			}

			static void Intersect(IntSet& result, const IntSet& left, const IntSet& right)
			{
				const auto shared = (std::min)(left.words.size(), right.words.size());
				result.words.resize(shared);
				for (std::size_t i = 0; i < shared; i++)
					result.words[i] = left.words[i] & right.words[i];
			}

			static void Subtract(IntSet& result, const IntSet& left, const IntSet& right)
			{
				result.words.resize(left.words.size());
				for (std::size_t i = 0; i < left.words.size(); i++)
				{
					const auto excluded = i < right.words.size() ? right.words[i] : 0u;
					result.words[i] = left.words[i] & ~excluded;
				}
			}

			static bool HasIntersection(const IntSet& left, const IntSet& right)
			{
				const auto shared = (std::min)(left.words.size(), right.words.size());
				for (std::size_t i = 0; i < shared; i++)
				{
					if ((left.words[i] & right.words[i]) != 0)
						return true;
				}
				return false;
			}
		};
	}
}

#endif
