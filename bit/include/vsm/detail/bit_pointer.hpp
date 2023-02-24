#pragma once

#include <vsm/assert.h>

#include <cstddef>

namespace vsm {
namespace detai::bits_ {

template<typename Word>
class bit_pointer;

template<typename Word>
class bit_reference;


template<typename Word>
class bit_pointer_base
{
	static_assert(std::is_unsigned_v<Word>);
	using word_type = std::remove_cv_t<Word>;

	static constexpr size_t word_bits = sizeof(Word) * CHAR_BIT;

protected:
	Word* m_word;
	size_t m_index;

public:
	explicit constexpr bit_pointer_base(Word* const word, size_t const index)
		: m_word(word)
		, m_index(index)
	{
		vsm_assert(index < word_bits);
	}

protected:
	bit_pointer_base() = default;
};

template<typename Word>
class bit_pointer final : bit_pointer_base<Word>
{
	using base = bit_pointer_base<Word>;
	static_assert(sizeof(ptrdiff_t) == sizeof(size_t));

public:
	using base::base;

	bit_pointer() = default;

	constexpr bit_pointer& operator=(bit_pointer const&) & = default;


	[[nodiscard]] constexpr bit_reference<Word> operator*() const noexcept
	{
		return bit_reference<Word>(base::m_word, base::m_index);
	}

	[[nodiscard]] constexpr bit_reference<Word> operator[](ptrdiff_t const offset) const noexcept
	{
		bit_pointer ptr = *this;
		ptr.advance(static_cast<size_t(offset));
		return bit_reference<Word>(ptr.m_word, ptr.m_index);
	}


	constexpr bit_pointer& operator++() & noexcept
	{
		advance(1);
		return *this;
	}

	[[nodiscard]] constexpr bit_pointer operator++(int) & noexcept
	{
		auto result = *this;
		advance(1);
		return result;
	}

	constexpr bit_pointer& operator--() & noexcept
	{
		advance(static_cast<size_t>(-1));
		return *this;
	}

	[[nodiscard]] constexpr bit_pointer operator--(int) & noexcept
	{
		auto result = *this;
		advance(static_cast<size_t>(-1));
		return result;
	}

	constexpr bit_pointer& operator+=(ptrdiff_t const offset) & noexcept
	{
		advance(static_cast<size_t>(offset));
		return *this;
	}

	constexpr bit_pointer& operator-=(ptrdiff_t const offset) & noexcept
	{
		advance(static_cast<size_t>(-offset));
		return *this;
	}

	[[nodiscard]] friend constexpr bit_pointer operator+(bit_pointer ptr, ptrdiff_t const offset) noexcept
	{
		ptr.advance(static_cast<size_t>(offset));
		return ptr;
	}

	[[nodiscard]] friend constexpr bit_pointer operator+(ptrdiff_t const offset, bit_pointer ptr) noexcept
	{
		ptr.advance(static_cast<size_t>(offset));
		return ptr;
	}

	[[nodiscard]] friend constexpr bit_pointer operator-(bit_pointer ptr, ptrdiff_t const offset) noexcept
	{
		ptr.advance(static_cast<size_t>(-offset));
		return ptr;
	}

	[[nodiscard]] friend constexpr ptrdiff_t operator-(bit_pointer const& lhs, bit_pointer const& rhs) noexcept
	{
		return
			(lhs.m_word - rhs.m_word) * static_cast<ptrdiff_t>(base::word_bits) +
			(static_cast<ptrdiff_t>(lhs.m_index) - static_cast<ptrdiff_t>(rhs.m_index));
	}

	[[nodiscard]] auto operator<=>(bit_pointer const&) const = default;

private:
	constexpr void advance(size_t const offset) noexcept
	{
		base::m_index += offset;
		base::m_word += base::m_index / base::word_bits;
		base::m_index %= base::word_bits;
	}
};


template<typename Word>
class bit_reference_base : bit_pointer_base<Word>
{
	using base = bit_pointer_base<Word>;
	using word_type = typename base::word_type;

public:
	using base::base;


	[[nodiscard]] constexpr bool operator() const noexcept
	{
		return *base::m_word & (static_cast<word_type>(1) << base::m_index);
	}

	[[nodiscard]] constexpr bit_pointer<Word> operator&() const noexcept
	{
		return bit_pointer<Word>(base::m_word, base::m_index);
	}


	[[nodiscard]] constexpr bool operator==(bit_reference_base const& other) const noexcept
	{
		return static_cast<bool>(*this) == static_cast<bool>(other);
	}

	[[nodiscard]] constexpr auto operator<=>(bit_reference_base const& other) const noexcept
	{
		return static_cast<bool>(*this) <=> static_cast<bool>(other);
	}
};

template<typename Word>
class bit_reference final : public bit_reference_base<Word>
{
	using base = bit_reference_base<Word>;
	using word_type = typename base::word_type;

public:
	using base::base;

	constexpr bit_reference const& operator=(bit_reference const& other) const noexcept
	{
		return *this = static_cast<bool>(other);
	}

	constexpr bit_reference const& operator=(bool const value) const noexcept
	{
		word_type const mask = static_cast<word_type>(1) << base::m_index;
		word_type const word = *base::m_word;
		*base::m_word = value ? word | mask : word & ~mask;
		return *this;
	}
};

template<typename Word> requires std::is_const_v<Word>
class bit_reference final : public bit_reference_base<Word>
{
	using base = bit_reference_base<Word>;

public:
	using base::base;

	constexpr bit_reference(bit_reference<std::remove_const_t<Word>> const& other) noexcept
		: base(static_cast<bit_reference_base<std::remove_const_t<Word> const&>(other))
	{
	}

	bit_reference& operator=(bit_reference const&) = delete;
	bit_reference& operator=(bool) = delete;
};

} // namespace detail::bits_

} // namespace vsm
