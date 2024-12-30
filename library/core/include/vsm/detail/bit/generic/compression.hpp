#pragma once

#include <concepts>
#include <limits>

namespace vsm::detail::generic {

template<std::unsigned_integral T>
[[nodiscard]] constexpr T bit_compress(T const value, T const mask) noexcept
{
	T result = 0;
	for (int i = 0, j = 0; i < std::numeric_limits<T>::digits; ++i)
	{
		bool const mask_bit = (mask >> i) & 1;
		result |= (mask_bit & (value >> i)) << j;
		j += mask_bit;
	}
	return result;
}

template<std::unsigned_integral T>
[[nodiscard]] constexpr T bit_expand(T const value, T const mask) noexcept
{
	T result = 0;
	for (int i = 0, j = 0; i < std::numeric_limits<T>::digits; ++i)
	{
		bool const mask_bit = (mask >> i) & 1;
		result |= (mask_bit & (value >> j)) << i;
		j += mask_bit;
	}
	return result;
}

} // namespace vsm::detail::generic
