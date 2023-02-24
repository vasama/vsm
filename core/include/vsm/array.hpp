#pragma once

#include <array>
#include <type_traits>

#include <cstddef>

namespace vsm {
namespace detail {

template<typename T, size_t Size>
struct std_array_layout
{
	T data[Size];
};

template<typename T>
struct std_array_layout<T, 0>
{
};

} // namespace detail

template<typename T, size_t Size>
#if __cpp_lib_is_layout_compatible
	requires std::is_layout_compatible_v<std::array<T, Size>, detail::std_array_layout<T, Size>>
#else
	requires (sizeof(std::array<T, Size>) == sizeof(detail::std_array_layout<T, Size>))
#endif
using array = std::array<T, Size>;

} // namespace vsm
