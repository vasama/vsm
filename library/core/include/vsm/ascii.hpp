#pragma once

#include <vsm/assert.h>
#include <vsm/concepts.hpp>
#include <vsm/standard.hpp>

#include <compare>
#include <span>
#include <string_view>

namespace vsm {

// P3688

template<character Char>
[[nodiscard]] constexpr bool ascii_is_any(Char const character) noexcept
{
	return character <= static_cast<Char>(0x7F);
}

template<character Char>
[[nodiscard]] constexpr bool ascii_is_digit(Char const character) noexcept
{
	return static_cast<Char>('0') <= character && character <= static_cast<Char>('9');
}

template<character Char>
[[nodiscard]] constexpr bool ascii_is_bit(Char const character) noexcept
{
	return character == static_cast<Char>('0') || character == static_cast<Char>('1');
}

template<character Char>
[[nodiscard]] constexpr bool ascii_is_octal_digit(Char const character) noexcept
{
	return static_cast<Char>('0') <= character && character <= static_cast<Char>('7');
}

template<character Char>
[[nodiscard]] constexpr bool ascii_is_hex_digit(Char const character) noexcept
{
	return
		(static_cast<Char>('0') <= character && character <= static_cast<Char>('9')) ||
		(static_cast<Char>('a') <= character && character <= static_cast<Char>('f')) ||
		(static_cast<Char>('A') <= character && character <= static_cast<Char>('F'));
}

template<character Char>
[[nodiscard]] constexpr bool ascii_is_lower(Char const character) noexcept
{
	return static_cast<Char>('a') <= character && character <= static_cast<Char>('z');
}

template<character Char>
[[nodiscard]] constexpr bool ascii_is_upper(Char const character) noexcept
{
	return static_cast<Char>('A') <= character && character <= static_cast<Char>('Z');
}

template<character Char>
[[nodiscard]] constexpr bool ascii_is_alphabetic(Char const character) noexcept
{
	return
		(static_cast<Char>('a') <= character && character <= static_cast<Char>('z')) ||
		(static_cast<Char>('A') <= character && character <= static_cast<Char>('Z'));
}

template<character Char>
[[nodiscard]] constexpr bool ascii_is_alphanumeric(Char const character) noexcept
{
	return
		(static_cast<Char>('0') <= character && character <= static_cast<Char>('9')) ||
		(static_cast<Char>('a') <= character && character <= static_cast<Char>('z')) ||
		(static_cast<Char>('A') <= character && character <= static_cast<Char>('Z'));
}

template<character Char>
[[nodiscard]] constexpr bool ascii_is_punctuation(Char character) noexcept;

template<character Char>
[[nodiscard]] constexpr bool ascii_is_graphic(Char character) noexcept;

template<character Char>
[[nodiscard]] constexpr bool ascii_is_printing(Char character) noexcept;

template<character Char>
[[nodiscard]] constexpr bool ascii_is_horizontal_whitespace(Char const character) noexcept
{
	return character == static_cast<Char>(' ') || character == static_cast<Char>('\t');
}

template<character Char>
[[nodiscard]] constexpr bool ascii_is_whitespace(Char const character) noexcept
{
	switch (character)
	{
	case static_cast<Char>(' '):
	case static_cast<Char>('\f'):
	case static_cast<Char>('\n'):
	case static_cast<Char>('\r'):
	case static_cast<Char>('\t'):
	case static_cast<Char>('\v'):
		return true;

	default:
		return false;
	}
}

template<character Char>
[[nodiscard]] constexpr bool ascii_is_control(Char const character) noexcept
{
	return
		(static_cast<Char>(0) <= character && character <= static_cast<Char>(0x1F)) ||
		character == static_cast<Char>(0x7F);
}

template<character Char>
[[nodiscard]] constexpr Char ascii_to_lower(Char const character) noexcept
{
	uint32_t unsigned_character = static_cast<uint32_t>(character);
	uint32_t const alpha_offset = unsigned_character - 'A';

	if (alpha_offset <= static_cast<uint32_t>('z' - 'a'))
	{
		unsigned_character += static_cast<uint32_t>('a' - 'A');
	}

	return static_cast<Char>(unsigned_character);
}

template<character Char>
[[nodiscard]] constexpr Char ascii_to_upper(Char const character) noexcept
{
	uint32_t unsigned_character = static_cast<uint32_t>(character);
	uint32_t const alpha_offset = unsigned_character - 'a';

	if (alpha_offset <= static_cast<uint32_t>('z' - 'a'))
	{
		unsigned_character += static_cast<uint32_t>('A' - 'a');
	}

	return static_cast<Char>(unsigned_character);
}

template<character Char>
[[nodiscard]] constexpr bool ascii_case_insensitive_equal(Char const lhs, Char const rhs) noexcept
{
	return ascii_to_upper(lhs) == ascii_to_upper(rhs);
}

template<character Char>
[[nodiscard]] constexpr std::strong_ordering ascii_case_insensitive_compare(
	Char const lhs,
	Char const rhs) noexcept
{
	return ascii_to_upper(lhs) <=> ascii_to_upper(rhs);
}


// Non-standard

namespace detail {

void _ascii_to_lower_fast(char const* beg, char const* end, char* out) noexcept;
void _ascii_to_upper_fast(char const* beg, char const* end, char* out) noexcept;

[[nodiscard]] std::strong_ordering _ascii_ci_cmp_fast(
	char const* lhs_beg,
	char const* lhs_end,
	char const* rhs_beg,
	char const* rhs_end) noexcept;

template<typename Char>
void _ascii_to_lower_impl(Char* beg, Char* const end, Char* out) noexcept
{
	while (beg != end)
	{
		*out++ = ascii_to_lower(*beg++);
	}
}

template<typename Char>
void _ascii_to_upper_impl(Char* beg, Char* const end, Char* out) noexcept
{
	while (beg != end)
	{
		*out++ = ascii_to_upper(*beg++);
	}
}

template<typename Char>
[[nodiscard]] std::strong_ordering _ascii_ci_cmp_impl(
	Char const* lhs_beg,
	Char const* const lhs_end,
	Char const* rhs_beg,
	Char const* const rhs_end) noexcept
{
	while (lhs_beg != lhs_end && rhs_beg != rhs_end)
	{
		auto const cmp = *lhs_beg++ <=> *rhs_beg;

		if (cmp != 0)
		{
			return cmp;
		}
	}

	return (lhs_beg != lhs_end) <=> (rhs_beg != rhs_end);
}

} // namespace detail

template<character Char>
constexpr void ascii_to_lower(std::span<Char> const string) noexcept
{
	Char* const beg = string.data();
	Char* const end = string.data() + string.size();

	if constexpr (std::is_same_v<Char, char>)
	{
		vsm_if_consteval
		{
			return detail::_ascii_to_lower_impl(beg, end, beg);
		}
		else
		{
			return detail::_ascii_to_lower_fast(beg, end, beg);
		}
	}
	else
	{
		return detail::_ascii_to_lower_impl(beg, end, beg);
	}
}

template<character Char>
constexpr void ascii_to_lower(
	std::basic_string_view<Char> const string,
	std::span<Char> const buffer) noexcept
{
	vsm_assert(buffer.size() >= string.size()); //PRECONDITION

	Char const* const beg = string.data();
	Char const* const end = string.data() + string.size();
	Char* out = buffer.data();

	if constexpr (std::is_same_v<Char, char>)
	{
		vsm_if_consteval
		{
			return detail::_ascii_to_lower_impl(beg, end, out);
		}
		else
		{
			return detail::_ascii_to_lower_fast(beg, end, out);
		}
	}
	else
	{
		return detail::_ascii_to_lower_impl(beg, end, out);
	}
}

template<character Char>
constexpr void ascii_to_upper(std::span<Char> const string) noexcept
{
	Char* const beg = string.data();
	Char* const end = string.data() + string.size();

	if constexpr (std::is_same_v<Char, char>)
	{
		vsm_if_consteval
		{
			return detail::_ascii_to_upper_impl(beg, end, beg);
		}
		else
		{
			return detail::_ascii_to_upper_fast(beg, end, beg);
		}
	}
	else
	{
		return detail::_ascii_to_upper_impl(beg, end, beg);
	}
}

template<character Char>
constexpr void ascii_to_upper(
	std::basic_string_view<Char> const string,
	std::span<Char> const buffer) noexcept
{
	vsm_assert(buffer.size() >= string.size()); //PRECONDITION

	Char const* const beg = string.data();
	Char const* const end = string.data() + string.size();
	Char* out = buffer.data();

	if constexpr (std::is_same_v<Char, char>)
	{
		vsm_if_consteval
		{
			return detail::_ascii_to_upper_impl(beg, end, out);
		}
		else
		{
			return detail::_ascii_to_upper_fast(beg, end, out);
		}
	}
	else
	{
		return detail::_ascii_to_upper_impl(beg, end, out);
	}
}

template<character Char>
constexpr bool ascii_case_insensitive_equal(
	std::basic_string_view<Char> const lhs,
	std::basic_string_view<Char> const rhs) noexcept
{
	Char* const lhs_beg = lhs.data();
	Char* const lhs_end = lhs.data() + lhs.size();

	Char* const rhs_beg = rhs.data();
	Char* const rhs_end = rhs.data() + rhs.size();

	if (lhs_end - lhs_beg != rhs_end - rhs_beg)
	{
		return false;
	}

	if constexpr (std::is_same_v<Char, char>)
	{
		vsm_if_consteval
		{
			return detail::_ascii_ci_cmp_impl(lhs_beg, lhs_end, rhs_beg, rhs_end) == 0;
		}
		else
		{
			return detail::_ascii_ci_cmp_fast(lhs_beg, lhs_end, rhs_beg, rhs_end) == 0;
		}
	}
	else
	{
		return detail::_ascii_ci_cmp_impl(lhs_beg, lhs_end, rhs_beg, rhs_end) == 0;
	}
}

template<character Char>
[[nodiscard]] constexpr std::strong_ordering ascii_case_insensitive_compare(
	std::basic_string_view<Char> const lhs,
	std::basic_string_view<Char> const rhs) noexcept
{
	Char* const lhs_beg = lhs.data();
	Char* const lhs_end = lhs.data() + lhs.size();

	Char* const rhs_beg = rhs.data();
	Char* const rhs_end = rhs.data() + rhs.size();

	if constexpr (std::is_same_v<Char, char>)
	{
		vsm_if_consteval
		{
			return detail::_ascii_ci_cmp_impl(lhs_beg, lhs_end, rhs_beg, rhs_end);
		}
		else
		{
			return detail::_ascii_ci_cmp_fast(lhs_beg, lhs_end, rhs_beg, rhs_end);
		}
	}
	else
	{
		return detail::_ascii_ci_cmp_impl(lhs_beg, lhs_end, rhs_beg, rhs_end);
	}
}

} // namespace vsm
