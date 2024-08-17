#pragma once

#include <concepts>
#include <stdfloat>

#if __STDCPP_FLOAT16_T__
#	define vsm_detail_float16 std::float16_t
#else
#	include <vsm/platform.h>

#	if vsm_compiler_gnu
#		define vsm_detail_float16 _Float16
#	else
#		include <vsm/detail/float16.hpp>
#	endif
#endif

namespace vsm {

using float16_t = vsm_detail_float16;
using float32_t = float;
using float64_t = double;

template<typename Float>
concept floating_point = std::floating_point<Float> || std::same_as<Float, float16_t>;

} // namespace vsm

#undef vsm_detail_float16
