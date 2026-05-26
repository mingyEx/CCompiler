#ifndef CORE_LIB_BIT_INT_SET_H
#define CORE_LIB_BIT_INT_SET_H

#include <algorithm>
#include <bit>
#include <cstdint>
#include <vector>

#include "Common.h"
#include "Exception.h"
#include "IntSet.h"

namespace CoreLib
{
	namespace Basic
	{
		// A modern bitmap-backed integer set for compiler analyses.
		// It is optimized for dense integer ids and fast set operations.
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

			void EnsureNonNegative(int value) const
			{
				if (value < 0)
					throw InvalidOperationException(L"BitIntSet does not support negative values.");
			}

		public:
			BitIntSet() = default;

			explicit BitIntSet(int universe_size)
			{
				resize_for_universe(universe_size);
			}

			int universe_size() const
			{
				return static_cast<int>(words.size()) * bits_per_word;
			}

			// Compatibility with the legacy IntSet naming.
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

			void resize_for_universe(int universe_size)
			{
				if (universe_size < 0)
					throw InvalidOperationException(L"BitIntSet universe size cannot be negative.");

				const auto required_words =
					static_cast<std::size_t>((universe_size + bits_per_word - 1) / bits_per_word);
				words.resize(required_words, 0);
			}

			void Resize(int universe_size)
			{
				resize_for_universe(universe_size);
			}

			void SetMax(int universe_size)
			{
				resize_for_universe(universe_size);
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
				EnsureNonNegative(value);
				const auto word_index = WordIndex(value);
				if (word_index >= static_cast<int>(words.size()))
					words.resize(static_cast<std::size_t>(word_index + 1), 0);
				words[static_cast<std::size_t>(word_index)] |= BitMask(value);
			}

			void Add(int value)
			{
				insert(value);
			}

			void erase(int value)
			{
				if (value < 0)
					return;

				const auto word_index = WordIndex(value);
				if (word_index >= static_cast<int>(words.size()))
					return;
				words[static_cast<std::size_t>(word_index)] &= ~BitMask(value);
			}

			void Remove(int value)
			{
				erase(value);
			}

			bool contains(int value) const
			{
				if (value < 0)
					return false;

				const auto word_index = WordIndex(value);
				if (word_index >= static_cast<int>(words.size()))
					return false;
				return (words[static_cast<std::size_t>(word_index)] & BitMask(value)) != 0;
			}

			bool Contains(int value) const
			{
				return contains(value);
			}

			void union_with(const BitIntSet & other)
			{
				if (other.words.size() > words.size())
					words.resize(other.words.size(), 0);

				for (std::size_t i = 0; i < other.words.size(); i++)
					words[i] |= other.words[i];
			}

			void UnionWith(const BitIntSet & other)
			{
				union_with(other);
			}

			void UnionWith(const IntSet & other)
			{
				for (int value = 0; value < other.Size(); value++)
				{
					if (other.Contains(value))
						insert(value);
				}
			}

			void intersect_with(const BitIntSet & other)
			{
				const auto shared_count = (std::min)(words.size(), other.words.size());
				for (std::size_t i = 0; i < shared_count; i++)
					words[i] &= other.words[i];

				for (std::size_t i = shared_count; i < words.size(); i++)
					words[i] = 0;
			}

			void IntersectWith(const BitIntSet & other)
			{
				intersect_with(other);
			}

			void IntersectWith(const IntSet & other)
			{
				for (int value = 0; value < universe_size(); value++)
				{
					if (contains(value) && !other.Contains(value))
						erase(value);
				}
			}

			void subtract(const BitIntSet & other)
			{
				const auto shared_count = (std::min)(words.size(), other.words.size());
				for (std::size_t i = 0; i < shared_count; i++)
					words[i] &= ~other.words[i];
			}

			bool has_intersection(const BitIntSet & other) const
			{
				const auto shared_count = (std::min)(words.size(), other.words.size());
				for (std::size_t i = 0; i < shared_count; i++)
				{
					if ((words[i] & other.words[i]) != 0)
						return true;
				}
				return false;
			}

			std::vector<int> values() const
			{
				std::vector<int> result;
				result.reserve(static_cast<std::size_t>(count()));
				for (std::size_t word_index = 0; word_index < words.size(); word_index++)
				{
					auto word = words[word_index];
					while (word != 0)
					{
						const auto bit_index = std::countr_zero(word);
						result.push_back(
							static_cast<int>(word_index * bits_per_word + bit_index));
						word &= (word - 1);
					}
				}
				return result;
			}

			bool operator == (const BitIntSet & other) const
			{
				const auto max_size = (std::max)(words.size(), other.words.size());
				for (std::size_t i = 0; i < max_size; i++)
				{
					const auto lhs = i < words.size() ? words[i] : 0;
					const auto rhs = i < other.words.size() ? other.words[i] : 0;
					if (lhs != rhs)
						return false;
				}
				return true;
			}

			bool operator != (const BitIntSet & other) const
			{
				return !(*this == other);
			}

			static BitIntSet make_union(const BitIntSet & left, const BitIntSet & right)
			{
				BitIntSet result = left;
				result.union_with(right);
				return result;
			}

			static BitIntSet make_intersection(const BitIntSet & left, const BitIntSet & right)
			{
				BitIntSet result = left;
				result.intersect_with(right);
				return result;
			}

			static BitIntSet make_subtraction(const BitIntSet & left, const BitIntSet & right)
			{
				BitIntSet result = left;
				result.subtract(right);
				return result;
			}
		};
	}
}

#endif
