#pragma once

#include <vsm/concepts.hpp>

#include <compare>
#include <span>
#include <string_view>

namespace vsm {

// P3688

template<character Char>
constexpr bool ascii_is_any(Char const character) noexcept
{
	return character <= static_cast<Char>(0x7F);
}

template<character Char>
constexpr bool ascii_is_digit(Char const character) noexcept
{
	return static_cast<Char>('0') <= character && character <= static_cast<Char>('9');
}

template<character Char>
constexpr bool ascii_is_bit(Char const character) noexcept
{
	return character == static_cast<Char>('0') || character == static_cast<Char>('1');
}

template<character Char>
constexpr bool ascii_is_octal_digit(Char const character) noexcept
{
	return static_cast<Char>('0') <= character && character <= static_cast<Char>('7');
}

template<character Char>
constexpr bool ascii_is_hex_digit(Char const character) noexcept
{
	return
		(static_cast<Char>('0') <= character && character <= static_cast<Char>('9')) ||
		(static_cast<Char>('a') <= character && character <= static_cast<Char>('f')) ||
		(static_cast<Char>('A') <= character && character <= static_cast<Char>('F'));
}

template<character Char>
constexpr bool ascii_is_lower(Char const character) noexcept
{
	return static_cast<Char>('a') <= character && character <= static_cast<Char>('z');
}

template<character Char>
constexpr bool ascii_is_upper(Char const character) noexcept
{
	return static_cast<Char>('A') <= character && character <= static_cast<Char>('Z');
}

template<character Char>
constexpr bool ascii_is_alphabetic(Char const character) noexcept
{
	return
		(static_cast<Char>('a') <= character && character <= static_cast<Char>('z')) ||
		(static_cast<Char>('A') <= character && character <= static_cast<Char>('Z'));
}

template<character Char>
constexpr bool ascii_is_alphanumeric(Char const character) noexcept
{
	return
		(static_cast<Char>('0') <= character && character <= static_cast<Char>('9')) ||
		(static_cast<Char>('a') <= character && character <= static_cast<Char>('z')) ||
		(static_cast<Char>('A') <= character && character <= static_cast<Char>('Z'));
}

template<character Char>
constexpr bool ascii_is_punctuation(Char const character) noexcept;

template<character Char>
constexpr bool ascii_is_graphic(Char const character) noexcept;

template<character Char>
constexpr bool ascii_is_printing(Char const character) noexcept;

template<character Char>
constexpr bool ascii_is_horizontal_whitespace(Char const character) noexcept
{
	return character == static_cast<Char>(' ') || character == static_cast<Char>('\t');
}

template<character Char>
constexpr bool ascii_is_whitespace(Char const character) noexcept
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
constexpr bool ascii_is_control(Char const character) noexcept
{
	return
		(static_cast<Char>(0) <= character && character <= static_cast<Char>(0x1F)) ||
		character == static_cast<Char>(0x7F);
}

template<character Char>
constexpr Char ascii_to_lower(Char const character) noexcept
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
constexpr Char ascii_to_upper(Char const character) noexcept
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
constexpr bool ascii_case_insensitive_equal(Char const lhs, Char const rhs) noexcept
{
	return ascii_to_upper(lhs) == ascii_to_upper(rhs);
}

template<character Char>
constexpr std::strong_ordering ascii_case_insensitive_compare(Char const lhs, Char const rhs) noexcept
{
	return ascii_to_upper(lhs) <=> ascii_to_upper(rhs);
}


// Not proposed by P3688

template<character Char>
constexpr void ascii_to_lower(std::span<Char> const string) noexcept;

template<character Char>
constexpr void ascii_to_lower(
	std::basic_string_view<Char> const string,
	std::span<Char> const buffer) noexcept;

template<character Char>
constexpr void ascii_to_upper(std::span<Char> const string) noexcept;

template<character Char>
constexpr void ascii_to_upper(
	std::basic_string_view<Char> const string,
	std::span<Char> const buffer) noexcept;

template<character Char>
constexpr bool ascii_case_insensitive_equal(
	std::basic_string_view<Char> const lhs,
	std::basic_string_view<Char> const rhs) noexcept;

template<character Char>
constexpr std::strong_ordering ascii_case_insensitive_compare(
	std::basic_string_view<Char> const lhs,
	std::basic_string_view<Char> const rhs) noexcept;


extern template void ascii_to_lower<char>(std::span<char> string) noexcept;
extern template void ascii_to_lower<char>(
	std::basic_string_view<char> string,
	std::span<char> buffer) noexcept;

extern template void ascii_to_upper<char>(std::span<char> string) noexcept;
extern template void ascii_to_upper<char>(
	std::basic_string_view<char> string,
	std::span<char> buffer) noexcept;

extern template bool ascii_case_insensitive_equal<char>(
	std::string_view lhs,
	std::string_view rhs) noexcept;

extern template std::strong_ordering ascii_case_insensitive_compare<char>(
	std::string_view lhs,
	std::string_view rhs) noexcept;

} // namespace vsm
