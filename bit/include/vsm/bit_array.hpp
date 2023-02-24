#pragma once

#include <vsm/assert.h>
#include <vsm/bit_pointer.hpp>
#include <vsm/bit_reference.hpp>

#include <climits>
#include <cstddef>

namespace vsm {
namespace detail::bits_ {

template<typename Word, size_t Size>
class bit_array_storage
{
	Word data[Size];
};

template<typename Word>
class bit_array_storage<Word, 0>
{
	static constexpr Word* data = nullptr;
};

template<size_t Size, typename Word = unsigned int>
class bit_array
{
	static constexpr size_t word_bits = sizeof(Word) * CHAR_BIT;
	static constexpr size_t word_count = (Size + word_bits - 1) / word_bits;

	[[no_unique_address]] bit_array_storage<Word, word_count> m = {};

public:
	using       reference = bit_reference<      Word>;
	using const_reference = bit_reference<const Word>;

	using       pointer = bit_pointer<      Word>;
	using const_pointer = bit_pointer<const Word>;


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

	[[nodiscard]] reference operator[](size_t const index) noexcept
	{
		vsm_assert(index < Size);
		return pointer(m.data, 0)[index];
	}

	[[nodiscard]] const_reference operator[](size_t const index) const noexcept
	{
		vsm_assert(index < Size);
		return const_pointer(m.data, 0)[index];
	}
};

} // namespace detail::bits_

using detail::bits_::bit_array;

} // namespace vsm
