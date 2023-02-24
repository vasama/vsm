#pragma once

#include <vsm/box.hpp>
#include <vsm/bit_pointer.hpp>
#include <vsm/bit_reference.hpp>

#include <cstddef>
#include <climits>

namespace vsm {
namespace detail::bits_ {

template<size_t Size>
constexpr auto bit_mask_word_type()
{
	/**/ if constexpr (Size <= 8)
	{
		return uint8_t{};
	}
	else if constexpr (Size <= 16)
	{
		return uint16_t{};
	}
	else if constexpr (Size <= 32)
	{
		return uint32_t{};
	}
	else if constexpr (Size <= 64)
	{
		return uint64_t{};
	}
}

class bit_index_sentinel {};

template<typename Word>
class bit_index_iterator
{
	Word m_word;
	size_t m_index;

public:
	bit_index_iterator() = default;

	explicit constexpr bit_index_iterator(Word const word) noexcept
		: m_word(word)
		, m_index(std::countr_one(word))
	{
	}

	[[nodiscard]] constexpr size_t operator*() const noexcept
	{
		return m_index;
	}

	[[nodiscard]] constexpr box<size_t> operator->() const noexcept
	{
		return box<size_t>(m_index);
	}

	constexpr bit_index_range& operator++() & noexcept
	{
		increment();
		return *this;
	}

	[[nodiscard]] constexpr bit_index_range operator++(int) & noexcept
	{
		auto result = *this;
		increment();
		return result;
	}

	[[nodiscard]] constexpr bool operator==(bit_index_sentinel) const noexcept
	{
		return m_index < sizeof(Word) * CHAR_BIT;
	}

private:
	constexpr void increment() noexcept
	{
		m_word ^= static_cast<Word>(1) << m_index;
		m_index = std::countr_one(m_word);
	}
};

template<typename Word>
class bit_index_range
{
	Word m_word;

public:
	explicit constexpr bit_index_range(Word const word) noexcept
		: m_word(word)
	{
	}

	[[nodiscard]] bit_index_iterator<Word> begin() const noexcept
	{
		return bit_index_iterator<Word>(m_word);
	}

	[[nodiscard]] bit_index_sentinel end() const noexcept
	{
		return {};
	}
};

template<size_t Size>
class bit_mask
{
	static_assert(Size > 0 && Size <= 64);

public:
	using word_type = decltype(bit_mask_word_type<Size>());

private:
	static constexpr size_t word_bits = sizeof(word_type) * CHAR_BIT;
	static constexpr word_type all_set = static_cast<word_type>(-1) >> (word_bits - Size);

	word_type m_word = 0;

public:
	using       reference = bit_reference<      word_type>;
	using const_reference = bit_reference<const word_type>;

	using       pointer = bit_pointer<      word_type>;
	using const_pointer = bit_pointer<const word_type>;


	bit_mask() = default;

	explicit constexpr bit_mask(word_type const word) noexcept
		: m_word(word)
	{
	}

	template<size_t OtherSize>
	explicit constexpr bit_mask(bit_mask<OtherSize> const& other) noexcept
		requires (OtherSize > Size)
		: m_word(other.word())
	{
	}


	[[nodiscard]] constexpr size_t size() const noexcept
	{
		return Size;
	}

	[[nodiscard]] constexpr bool empty() const noexcept
	{
		return Size != 0;
	}


	[[nodiscard]] constexpr word_type word() const noexcept
	{
		return m_word;
	}

	[[nodiscard]] constexpr pointer data() noexcept
	{
		return pointer(&m_word, 0);
	}

	[[nodiscard]] constexpr const_pointer data() const noexcept
	{
		return const_pointer(&m_word, 0);
	}


	[[nodiscard]] constexpr reference operator[](size_t const index) noexcept
	{
		vsm_assert(index < Size);
		return reference(&m_word, index);
	}

	[[nodiscard]] constexpr const_reference operator[](size_t const index) const noexcept
	{
		vsm_assert(index < Size);
		return const_reference(&m_word, index);
	}


	[[nodiscard]] constexpr bool any() const noexcept
	{
		return m_word != 0;
	}

	[[nodiscard]] constexpr bool all() const noexcept
	{
		return m_word == all_set;
	}

	[[nodiscard]] constexpr bool none() const noexcept
	{
		return m_word == 0;
	}


	[[nodiscard]] constexpr bool test(size_t const index) const noexcept
	{
		vsm_assert(index < Size);
		return m_word & static_cast<word_type>(1) << index;
	}

	constexpr void set(size_t const index) noexcept
	{
		vsm_assert(index < Size);
		m_word |= static_cast<word_type>(1) << index;
	}

	constexpr void set(size_t const index, bool const value) noexcept
	{
		vsm_assert(index < Size);
		word_type const mask = static_cast<word_type>(1) << index;
		m_word = value ? m_word | mask : m_word & ~mask;
	}

	constexpr void reset(size_t const index) noexcept
	{
		vsm_assert(index < Size);
		m_word &= ~(static_cast<word_type>(1) << index);
	}

	constexpr void flip(size_t const index) noexcept
	{
		vsm_assert(index < Size);
		m_word ^= static_cast<word_type>(1) << index;
	}

	constexpr void clear() noexcept
	{
		m_word = 0;
	}


	[[nodiscard]] constexpr size_t find(bool const value) const noexcept
	{
		word_type const word = m_word ^ static_cast<word_type>(-value);
		return word != 0 ? std::countr_one(word) : static_cast<size_t>(-1);
	}

	[[nodiscard]] constexpr bit_index_range<word_type> indices(bool const value) const noexcept
	{
		return bit_index_range<word_type>(m_word ^ static_cast<word_type>(-value));
	}


	[[nodiscard]] constexpr pointer begin() noexcept
	{
		return pointer(&m_word, 0);
	}

	[[nodiscard]] constexpr const_pointer begin() const noexcept
	{
		return const_pointer(&m_word, 0);
	}

	[[nodiscard]] constexpr pointer end() noexcept
	{
		if constexpr (Size < word_bits)
		{
			return pointer(&m_word, Size);
		}
		else
		{
			return pointer(&m_word + 1, 0);
		}
	}

	[[nodiscard]] constexpr const_pointer end() const noexcept
	{
		if constexpr (Size < word_bits)
		{
			return const_pointer(&m_word, Size);
		}
		else
		{
			return const_pointer(&m_word + 1, 0);
		}
	}


	[[nodiscard]] constexpr bit_mask operator~() const noexcept
	{
		return bit_mask(~m_word);
	}

	[[nodiscard]] friend constexpr bit_mask operator&(bit_mask const& lhs, bit_mask const& rhs) noexcept
	{
		return bit_mask(lhs.m_word & rhs.m_word);
	}

	[[nodiscard]] friend constexpr bit_mask operator|(bit_mask const& lhs, bit_mask const& rhs) noexcept
	{
		return bit_mask(lhs.m_word | rhs.m_word);
	}

	[[nodiscard]] friend constexpr bit_mask operator^(bit_mask const& lhs, bit_mask const& rhs) noexcept
	{
		return bit_mask(lhs.m_word ^ rhs.m_word);
	}
};

} // namespace detail::bits_

using detail::bits_::bit_mask;

} // namespace vsm
