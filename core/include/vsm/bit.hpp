#pragma once

#include <vsm/platform.h>
#include <vsm/standard.hpp>

#include <concepts>

#if __has_include(<vsm/detail/bit/vsm_arch.hpp>)
#	include <vsm/detail/bit/compression/vsm_arch.hpp>
#else
#	define vsm_has_native_bit_compression 0
#endif

#include <vsm/detail/bit/generic/compression.hpp>

namespace vsm {

#if vsm_has_native_bit_compression
template<std::unsigned_integral T>
[[nodiscard]] vsm_always_inline constexpr T bit_compress(T const value, T const mask) noexcept
{
	vsm_if_consteval
	{
		return detail::generic::bit_compress(value, mask);
	}
	else
	{
		return detail::_bit_compress(value, mask);
	}
}

template<std::unsigned_integral T>
[[nodiscard]] vsm_always_inline constexpr T bit_expand(T const value, T const mask) noexcept
{
	vsm_if_consteval
	{
		return detail::generic::bit_expand(value, mask);
	}
	else
	{
		return detail::_bit_expand(value, mask);
	}
}
#else
using detail::generic::bit_compress;
using detail::generic::bit_expand;
#endif

} // namespace vsm
