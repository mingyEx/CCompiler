#ifndef COMPILER_INTERMEDIATE_BIT_INT_SET_H
#define COMPILER_INTERMEDIATE_BIT_INT_SET_H

#include <algorithm>
#include <bit>
#include <cstdint>
#include <vector>

#include "IntSet.h"

namespace Compiler
{
	namespace Intermediate
	{
		class BitIntSet
		{
		private:
			static constexpr int bits_per_word = 64;
			std::vector<std::uint64_t> words;

			static int WordIndex(int value)
			{
				return value / bits_per_word;
			}

			static std::uint64_t BitMask(int value)
			{
				return std::uint64_t{1} << (value % bits_per_word);
			}

		public:
			BitIntSet() = default;

			explicit BitIntSet(int universeSize)
			{
				resize_for_universe(universeSize);
			}

			int universe_size() const
			{
				return static_cast<int>(words.size()) * bits_per_word;
			}

			int Size() const
			{
				return universe_size();
			}

			bool empty() const
			{
				for (const auto word : words)
				{
					if (word != 0)
						return false;
				}
				return true;
			}

			int count() const
			{
				int total = 0;
				for (const auto word : words)
					total += std::popcount(word);
				return total;
			}

			void resize_for_universe(int universeSize)
			{
				const auto wordCount = static_cast<std::size_t>(((std::max)(universeSize, 0) + bits_per_word - 1) / bits_per_word);
				words.resize(wordCount, 0);
			}

			void Resize(int universeSize)
			{
				resize_for_universe(universeSize);
			}

			void SetMax(int universeSize)
			{
				resize_for_universe(universeSize);
				clear();
			}

			void clear()
			{
				std::fill(words.begin(), words.end(), std::uint64_t{0});
			}

			void Clear()
			{
				clear();
			}

			void insert(int value)
			{
				if (value < 0)
					return;

				const auto index = static_cast<std::size_t>(WordIndex(value));
				if (index >= words.size())
					words.resize(index + 1, 0);
				words[index] |= BitMask(value);
			}

			void Add(int value)
			{
				insert(value);
			}

			void erase(int value)
			{
				if (value < 0)
					return;

				const auto index = static_cast<std::size_t>(WordIndex(value));
				if (index < words.size())
					words[index] &= ~BitMask(value);
			}

			void Remove(int value)
			{
				erase(value);
			}

			bool contains(int value) const
			{
				if (value < 0)
					return false;

				const auto index = static_cast<std::size_t>(WordIndex(value));
				return index < words.size() && (words[index] & BitMask(value)) != 0;
			}

			bool Contains(int value) const
			{
				return contains(value);
			}

			void union_with(const BitIntSet& set)
			{
				if (set.words.size() > words.size())
					words.resize(set.words.size(), 0);
				for (std::size_t i = 0; i < set.words.size(); i++)
					words[i] |= set.words[i];
			}

			void UnionWith(const BitIntSet& set)
			{
				union_with(set);
			}

			void UnionWith(const IntSet& set)
			{
				for (int value = 0; value < set.Size(); value++)
				{
					if (set.Contains(value))
						insert(value);
				}
			}

			void intersect_with(const BitIntSet& set)
			{
				const auto shared = (std::min)(words.size(), set.words.size());
				for (std::size_t i = 0; i < shared; i++)
					words[i] &= set.words[i];
				for (std::size_t i = shared; i < words.size(); i++)
					words[i] = 0;
			}

			void IntersectWith(const BitIntSet& set)
			{
				intersect_with(set);
			}

			void IntersectWith(const IntSet& set)
			{
				for (int value = 0; value < universe_size(); value++)
				{
					if (contains(value) && !set.Contains(value))
						erase(value);
				}
			}

			void subtract(const BitIntSet& set)
			{
				const auto shared = (std::min)(words.size(), set.words.size());
				for (std::size_t i = 0; i < shared; i++)
					words[i] &= ~set.words[i];
			}

			bool has_intersection(const BitIntSet& set) const
			{
				const auto shared = (std::min)(words.size(), set.words.size());
				for (std::size_t i = 0; i < shared; i++)
				{
					if ((words[i] & set.words[i]) != 0)
						return true;
				}
				return false;
			}

			std::vector<int> values() const
			{
				std::vector<int> result;
				result.reserve(static_cast<std::size_t>(count()));
				for (std::size_t wordIndex = 0; wordIndex < words.size(); wordIndex++)
				{
					auto word = words[wordIndex];
					while (word != 0)
					{
						const auto bitIndex = std::countr_zero(word);
						result.push_back(static_cast<int>(wordIndex * bits_per_word + bitIndex));
						word &= (word - 1);
					}
				}
				return result;
			}

			bool operator == (const BitIntSet& set) const
			{
				const auto maxSize = (std::max)(words.size(), set.words.size());
				for (std::size_t i = 0; i < maxSize; i++)
				{
					const auto left = i < words.size() ? words[i] : 0;
					const auto right = i < set.words.size() ? set.words[i] : 0;
					if (left != right)
						return false;
				}
				return true;
			}

			bool operator != (const BitIntSet& set) const
			{
				return !(*this == set);
			}

			static BitIntSet make_union(const BitIntSet& left, const BitIntSet& right)
			{
				BitIntSet result = left;
				result.union_with(right);
				return result;
			}

			static BitIntSet make_intersection(const BitIntSet& left, const BitIntSet& right)
			{
				BitIntSet result = left;
				result.intersect_with(right);
				return result;
			}

			static BitIntSet make_subtraction(const BitIntSet& left, const BitIntSet& right)
			{
				BitIntSet result = left;
				result.subtract(right);
				return result;
			}
		};
	}
}

#endif
