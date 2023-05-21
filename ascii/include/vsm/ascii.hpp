#pragma once

#include <compare>
#include <span>
#include <string_view>

#include <cstddef>
#include <cstdint>

namespace vsm::ascii {

inline constexpr char to_lower(char const c)
{
	uint8_t u = static_cast<uint8_t>(c);
	uint8_t const m = u - static_cast<uint8_t>('A');
	if (m <= static_cast<uint8_t>('z' - 'a'))
	{
		u += static_cast<uint8_t>('a' - 'A');
	}
	return static_cast<char>(u);
}

inline constexpr char to_upper(char const c)
{
	uint8_t u = static_cast<uint8_t>(c);
	uint8_t const m = u - static_cast<uint8_t>('a');
	if (m <= static_cast<uint8_t>('z' - 'a'))
	{
		u += static_cast<uint8_t>('A' - 'a');
	}
	return static_cast<char>(u);
}

void to_lower_in_place(std::span<char> data);
void to_upper_in_place(std::span<char> data);

void to_lower(std::string_view string, std::span<char> buffer);
void to_upper(std::string_view string, std::span<char> buffer);

bool equal(std::string_view lhs, std::string_view rhs);
std::strong_ordering compare(std::string_view lhs, std::string_view rhs);
size_t hash(std::string_view string);

} // namespace vsm::ascii
