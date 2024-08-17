#pragma once

#include <vsm/assert.h>
#include <vsm/bit_ptr.hpp>
#include <vsm/standard.hpp>

#include <climits>
#include <cstddef>

namespace vsm {
namespace detail {

template<typename Word, size_t Size>
struct _bit_array_storage
{
	Word data[Size];
};

template<typename Word>
struct _bit_array_storage<Word, 0>
{
	static constexpr Word* data = nullptr;
};

} // namespace detail

template<bit_size_t Size, detail::_bit_word Word = unsigned int>
class bit_array
{
	static constexpr size_t word_bits = sizeof(Word) * CHAR_BIT;
	static constexpr size_t word_count = static_cast<size_t>((Size + (word_bits - 1)) / word_bits);

	vsm_no_unique_address detail::_bit_array_storage<Word, word_count> m = {};

public:
	using size_type = bit_size_t;
	using difference_type = bit_ptrdiff_t;

	using       reference = bit_ref<      Word>;
	using const_reference = bit_ref<const Word>;

	using       pointer = bit_ptr<      Word>;
	using const_pointer = bit_ptr<const Word>;

	using       iterator =       pointer;
	using const_iterator = const_pointer;


	[[nodiscard]] constexpr size_t size() const noexcept
	{
		return Size;
	}

	[[nodiscard]] constexpr bool empty() const noexcept
	{
		return Size != 0;
	}


	[[nodiscard]] pointer data() noexcept
	{
		return pointer(m.data, 0);
	}

	[[nodiscard]] const_pointer data() const noexcept
	{
		return const_pointer(m.data, 0);
	}

	[[nodiscard]] constexpr reference front() noexcept
	{
		static_assert(Size > 0);
		return *pointer(m.data, 0);
	}

	[[nodiscard]] constexpr const_reference front() const noexcept
	{
		static_assert(Size > 0);
		return *const_pointer(m.data, 0);
	}

	[[nodiscard]] constexpr reference back() noexcept
	{
		static_assert(Size > 0);
		return pointer(m.data, 0)[Size - 1];
	}

	[[nodiscard]] constexpr const_reference back() const noexcept
	{
		static_assert(Size > 0);
		return const_pointer(m.data, 0)[Size - 1];
	}

	[[nodiscard]] reference operator[](bit_size_t const index)
	{
		vsm_assert(index < Size);
		return pointer(m.data, 0)[index];
	}

	[[nodiscard]] const_reference operator[](bit_size_t const index) const
	{
		vsm_assert(index < Size);
		return const_pointer(m.data, 0)[index];
	}

	[[nodiscard]] iterator begin() noexcept
	{
		return pointer(m.data, 0);
	}

	[[nodiscard]] const_iterator begin() const noexcept
	{
		return const_pointer(m.data, 0);
	}

	[[nodiscard]] iterator end() noexcept
	{
		return pointer(m.data, 0) + Size;
	}

	[[nodiscard]] const_iterator end() const noexcept
	{
		return const_pointer(m.data, 0) + Size;
	}
};

} // namespace vsm
