#pragma once

#include <vsm/tag_invoke.hpp>
#include <vsm/type_traits.hpp>

namespace vsm {

template<typename T>
inline constexpr bool is_trivially_relocatable_v = std::is_trivially_copyable_v<T>;

} // namespace vsm
