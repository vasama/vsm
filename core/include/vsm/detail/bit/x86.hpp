#pragma once

#include <vsm/platform.h>
#include <vsm/x86.hpp>

namespace vsm::detail {

#define vsm_has_native_bit_compression 1

template<typename T>
vsm_always_inline T _bit_compress(T const value, T const mask) noexcept
{
	/**/ if constexpr (sizeof(T) <= sizeof(uint32_t))
	{
		return static_cast<T>(_pext_u32(static_cast<uint32_t>(value), static_cast<uint32_t>(mask)));
	}
	else if constexpr (sizeof(T) == sizeof(uint64_t))
	{
		return static_cast<T>(_pext_u32(static_cast<uint64_t>(value), static_cast<uint64_t>(mask)));
	}
	else
	{
		static_assert(sizeof(T) == 0);
	}
}

template<typename T>
vsm_always_inline T _bit_expand(T const value, T const mask) noexcept
{
	/**/ if constexpr (sizeof(T) <= sizeof(uint32_t))
	{
		return static_cast<T>(_pdep_u32(static_cast<uint32_t>(value), static_cast<uint32_t>(mask)));
	}
	else if constexpr (sizeof(T) == sizeof(uint64_t))
	{
		return static_cast<T>(_pdep_u64(static_cast<uint64_t>(value), static_cast<uint64_t>(mask)));
	}
	else
	{
		static_assert(sizeof(T) == 0);
	}
}

} // namespace vsm::detail
