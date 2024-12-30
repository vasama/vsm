#pragma once

#include <compare>
#include <span>
#include <string_view>

namespace vsm::ascii {

inline constexpr char to_lower(char const c)
{
	unsigned char u = static_cast<unsigned char>(c);
	unsigned char const m = u - static_cast<unsigned char>('A');
	if (m <= static_cast<unsigned char>('z' - 'a'))
	{
		u += static_cast<unsigned char>('a' - 'A');
	}
	return static_cast<char>(u);
}

inline constexpr char to_upper(char const c)
{
	unsigned char u = static_cast<unsigned char>(c);
	unsigned char const m = u - static_cast<unsigned char>('a');
	if (m <= static_cast<unsigned char>('z' - 'a'))
	{
		u += static_cast<unsigned char>('A' - 'a');
	}
	return static_cast<char>(u);
}

void to_lower(std::span<char> data);
void to_upper(std::span<char> data);

void to_lower(std::string_view string, std::span<char> buffer);
void to_upper(std::string_view string, std::span<char> buffer);

[[nodiscard]] bool equal(std::string_view lhs, std::string_view rhs);
[[nodiscard]] std::strong_ordering compare(std::string_view lhs, std::string_view rhs);
[[nodiscard]] size_t hash(std::string_view string);

} // namespace vsm::ascii
