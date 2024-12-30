#pragma once

#include <bit>

#ifdef __cpp_lib_byteswap

namespace vsm {
using std::byteswap;
} // namespace vsm

#else

#include <concepts>

namespace vsm {

template<std::integral T>
constexpr T byteswap(T const value) noexcept
{
	/**/ if constexpr (sizeof(T) == 1)
	{
		return value;
	}
	else if constexpr (sizeof(T) == 2)
	{
		return __builtin_bswap16(value);
	}
	else if constexpr (sizeof(T) == 4)
	{
		return __builtin_bswap32(value);
	}
	else if constexpr (sizeof(T) == 8)
	{
		return __builtin_bswap64(value);
	}
	else
	{
		static_assert(sizeof(T) == 0);
	}
}

} // namespace vsm

#endif
