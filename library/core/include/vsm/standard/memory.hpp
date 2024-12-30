#pragma once

#include <memory>

#ifdef __cpp_lib_is_sufficiently_aligned

namespace vsm {
using std::is_sufficiently_aligned;
} // namespace vsm

#else // __cpp_lib_is_sufficiently_aligned

#include <bit>

namespace vsm {

template<size_t Alignment, typename T>
[[nodiscard]] bool is_sufficiently_aligned(T* const ptr)
{
	return std::bit_cast<uintptr_t>(ptr) % Alignment == 0;
}

#endif // __cpp_lib_is_sufficiently_aligned

} // namespace vsm
