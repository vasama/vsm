#pragma once

#include <vsm/platform.h>

#if vsm_compiler_msvc
#	include <__msvc_int128.hpp>
#endif

namespace vsm {

#if vsm_compiler_msvc
using int128_t = std::_Signed128;
using uint128_t = std::_Unsigned128;
#else
using int128_t = __int128_t;
using uint128_t = __uint128_t;
#endif

} // namespace vsm
