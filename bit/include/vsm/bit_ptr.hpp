#pragma once

#include <vsm/assert.h>
#include <vsm/concepts.hpp>

#include <compare>
#include <iterator>

#include <climits>
#include <cstddef>
#include <cstdint>

namespace vsm {

#  if vsm_word_bits <= 12
using bit_size_t = uint_least16_t;
using bit_ptrdiff_t = int_least16_t;
#elif vsm_word_bits <= 28
using bit_size_t = uint_least32_t;
using bit_ptrdiff_t = int_least32_t;
#elif vsm_word_bits <= 60
using bit_size_t = uint_least64_t;
using bit_ptrdiff_t = int_least64_t;
#else
using bit_size_t = size_t;
using bit_ptrdiff_t = ptrdiff_t;
#endif

namespace detail {

template<typename T>
concept _bit_word =
	std::unsigned_integral<T> &&
	not_same_as<T, bool>;

#ifndef __INTELLISENSE__
template<typename T>
concept _bit_offset =
	std::unsigned_integral<T> &&
	requires (T x) { bit_size_t{ x }; };
#endif

} // namespace detail

template<detail::_bit_word Word>
class bit_ptr;

template<detail::_bit_word Word>
class bit_ref;

template<detail::_bit_word Word>
class bit_ptr
{
	static constexpr size_t word_bits = sizeof(Word) * CHAR_BIT;

	Word* m_ptr;
	size_t m_pos;

public:
	bit_ptr() = default;

	explicit constexpr bit_ptr(Word* const ptr, size_t const offset = 0)
		: m_ptr(ptr)
		, m_pos(offset)
	{
	}

	template<not_same_as<Word> U>
		requires std::convertible_to<U*, Word*>
	constexpr bit_ptr(bit_ptr<U> const& other) noexcept
		: m_ptr(other.m_ptr)
		, m_pos(other.m_pos)
	{
	}

	bit_ptr& operator=(bit_ptr const&) & = default;


	[[nodiscard]] constexpr bit_ref<Word> operator*() const
	{
		return bit_ref<Word>(*m_ptr, m_pos);
	}

#ifndef __INTELLISENSE__
	[[nodiscard]] constexpr bit_ref<Word> operator[](detail::_bit_offset auto const offset) const
	{
		auto ptr = m_ptr;
		auto pos = m_pos;
		add_unsigned(ptr, pos, offset);
		return bit_ref<Word>(*ptr, pos);
	}
#endif

	[[nodiscard]] constexpr bit_ref<Word> operator[](bit_ptrdiff_t const offset) const
	{
		auto ptr = m_ptr;
		auto pos = m_pos;
		add_signed(ptr, pos, offset);
		return bit_ref<Word>(*ptr, pos);
	}


	constexpr bit_ptr& operator++() &
	{
		increment(m_ptr, m_pos);
		return *this;
	}

	[[nodiscard]] constexpr bit_ptr operator++(int) &
	{
		bit_ptr r = *this;
		increment(m_ptr, m_pos);
		return r;
	}

	constexpr bit_ptr& operator--() &
	{
		decrement(m_ptr, m_pos);
		return *this;
	}

	[[nodiscard]] constexpr bit_ptr operator--(int) &
	{
		bit_ptr r = *this;
		decrement(m_ptr, m_pos);
		return r;
	}

#ifndef __INTELLISENSE__
	constexpr bit_ptr& operator+=(detail::_bit_offset auto const offset)&
	{
		add_unsigned(m_ptr, m_pos, offset);
		return *this;
	}
#endif

	constexpr bit_ptr& operator+=(ptrdiff_t const offset) &
	{
		add_signed(m_ptr, m_pos, offset);
		return *this;
	}

#ifndef __INTELLISENSE__
	constexpr bit_ptr& operator-=(detail::_bit_offset auto const offset)&
	{
		sub_unsigned(m_ptr, m_pos, offset);
		return *this;
	}
#endif

	constexpr bit_ptr& operator-=(ptrdiff_t const offset) &
	{
		add_signed(m_ptr, m_pos, -offset);
		return *this;
	}

#ifndef __INTELLISENSE__
	[[nodiscard]] friend constexpr bit_ptr operator+(bit_ptr ptr, detail::_bit_offset auto const offset)
	{
		add_unsigned(ptr.m_ptr, ptr.m_pos, offset);
		return ptr;
	}
#endif

	[[nodiscard]] friend constexpr bit_ptr operator+(bit_ptr ptr, bit_ptrdiff_t const offset)
	{
		add_signed(ptr.m_ptr, ptr.m_pos, offset);
		return ptr;
	}

#ifndef __INTELLISENSE__
	[[nodiscard]] friend constexpr bit_ptr operator+(detail::_bit_offset auto const offset, bit_ptr ptr)
	{
		add_unsigned(ptr.m_ptr, ptr.m_pos, offset);
		return ptr;
	}
#endif

	[[nodiscard]] friend constexpr bit_ptr operator+(bit_ptrdiff_t const offset, bit_ptr ptr)
	{
		add_signed(ptr.m_ptr, ptr.m_pos, offset);
		return ptr;
	}

#ifndef __INTELLISENSE__
	[[nodiscard]] friend constexpr bit_ptr operator-(bit_ptr ptr, detail::_bit_offset auto const offset)
	{
		sub_unsigned(ptr.m_ptr, ptr.m_pos, offset);
		return ptr;
	}
#endif

	[[nodiscard]] friend constexpr bit_ptr operator-(bit_ptr ptr, bit_ptrdiff_t const offset)
	{
		add_signed(ptr.m_ptr, ptr.m_pos, -offset);
		return ptr;
	}

	[[nodiscard]] friend constexpr bit_ptrdiff_t operator-(bit_ptr const& lhs, bit_ptr const& rhs)
	{
		return static_cast<bit_ptrdiff_t>(lhs.m_ptr - rhs.m_ptr) * word_bits + (lhs.m_pos - rhs.m_pos);
	}

	[[nodiscard]] auto operator<=>(bit_ptr const&) const = default;

private:
	static constexpr void increment(Word*& ptr, size_t& pos)
	{
		if (++pos == word_bits)
		{
			++ptr;
			pos = 0;
		}
	}

	static constexpr void decrement(Word*& ptr, size_t& pos)
	{
		if (pos-- == 0)
		{
			--ptr;
			pos = word_bits - 1;
		}
	}

	static constexpr void add_unsigned(Word*& ptr, size_t& pos, bit_size_t offset)
	{
		offset += pos;
		ptr += static_cast<size_t>(offset / word_bits);
		pos = static_cast<size_t>(offset % word_bits);
	}

	static constexpr void sub_unsigned(Word*& ptr, size_t& pos, bit_size_t offset)
	{
		//TODO: Maybe this can be slightly optimised.
		add_signed(ptr, pos, -static_cast<bit_ptrdiff_t>(offset));
	}

	static constexpr void add_signed(Word*& ptr, size_t& pos, bit_ptrdiff_t offset)
	{
		offset += pos;
		ptr += static_cast<ptrdiff_t>(offset / static_cast<ptrdiff_t>(word_bits));
		auto mod = static_cast<ptrdiff_t>(offset % static_cast<ptrdiff_t>(word_bits));
		if (mod < 0)
		{
			mod += static_cast<ptrdiff_t>(word_bits);
			--ptr;
		}
		pos = static_cast<size_t>(mod);
	}

	template<detail::_bit_word U>
	friend class bit_ptr;
};

template<detail::_bit_word Word>
class bit_ref
{
	static constexpr size_t word_bits = sizeof(Word) * CHAR_BIT;

	using word_type = remove_cv_t<Word>;

	Word& m_ref;
	word_type m_mask;

public:
	explicit constexpr bit_ref(Word& ref, size_t const offset)
		: m_ref(ref)
		, m_mask(static_cast<word_type>(static_cast<word_type>(1) << offset))
	{
		vsm_assert(offset < word_bits);
	}

	template<not_same_as U>
		requires std::convertible_to<U*, Word*>
	constexpr bit_ref(bit_ref<U> const& other)
		: m_ref(other.m_ref)
		, m_mask(other.m_mask)
	{
	}

	[[nodiscard]] constexpr operator bool() const noexcept
	{
		return (m_ref & m_mask) != 0;
	}

	constexpr bit_ref const& operator=(bool const value) const noexcept
	{
		m_ref = value ? m_ref | m_mask : m_ref & ~m_mask;
		return *this;
	}

	template<typename U>
		requires std::convertible_to<U*, Word const*>
	constexpr bit_ref const& operator=(bit_ref<U> const& other) const noexcept
	{
		m_ref = other ? m_ref | m_mask : m_ref & ~m_mask;
		return *this;
	}

private:
	template<detail::_bit_word U>
	friend class bit_ref;
};

} // namespace vsm

template<typename T>
struct std::iterator_traits<vsm::bit_ptr<T>>
{
	using difference_type = vsm::bit_ptrdiff_t;
	using value_type = T;
	using pointer = T*;
	using reference = T&;
	using iterator_category = random_access_iterator_tag;
};
