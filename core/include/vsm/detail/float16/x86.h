#pragma once

#include <vsm/platform.h>

#if vsm_compiler_msvc
inline unsigned short vsm_detail_float_to_half(float const x)
{
	return static_cast<unsigned short>(
		_mm_cvtsi128_si32(
			_mm_cvtps_ph(
				_mm_set_ss(x),
				_MM_FROUND_CUR_DIRECTION)));
}

inline float vsm_detail_half_to_float(unsigned short const x)
{
	return _mm_cvtss_f32(_mm_cvtph_ps(_mm_cvtsi32_si128(x)));
}
#else
#define vsm_detail_float_to_half(...) \
	(_cvtss_sh((__VA_ARGS__), _MM_FROUND_CUR_DIRECTION))

#define vsm_detail_half_to_float(...) \
	(_cvtsh_ss((__VA_ARGS__)))
#endif

#define vsm_detail_half_promotion 1
