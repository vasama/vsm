#pragma once

#include <vsm/platform.h>

#if __cplusplus
extern "C" {
#endif

/// @brief Assertion failure handler. Can be replaced by the user.
/// @param file Name of the source file where the assertion failed.
/// @param line Line in the source file where the assertion failed.
/// @param expr The failed assertion expression.
/// @return Returns true if the program should raise a debuggable interrupt at the callsite.
bool vsm_assert_fail(char const* file, int line, char const* expr);

#if __cplusplus
constexpr
#endif
inline bool vsm_detail_assert_args(bool const value)
{
	return value;
}

#define vsm_detail_assert_fail(...) ( \
		vsm_assert_fail(__FILE__, __LINE__, #__VA_ARGS__) \
			? vsm_debugbreak() \
			: (void)0 \
	)

#define vsm_detail_assert(...) ( \
		vsm_detail_assert_args(__VA_ARGS__) \
			? (void)0 \
			: vsm_detail_assert_fail(__VA_ARGS__) \
	)

#if vsm_config_assert > 0
#	define vsm_assert(...) vsm_detail_assert(__VA_ARGS__)
#	define vsm_assume(...) vsm_detail_assert(__VA_ARGS__)
#	define vsm_verify(...) vsm_detail_assert(__VA_ARGS__)
#else
#	define vsm_assert(...) ((void)0)
#	define vsm_assume(...) vsm_platform_assume(__VA_ARGS__)
#	define vsm_verify(...) ((void)(__VA_ARGS__))
#endif

#if vsm_config_assert > 1
#	define vsm_assert_slow(...) vsm_detail_assert(__VA_ARGS__)
#	define vsm_assume_slow(...) vsm_detail_assert(__VA_ARGS__)
#	define vsm_verify_slow(...) vsm_detail_assert(__VA_ARGS__)
#else
#	define vsm_assert_slow(...) ((void)0)
#	define vsm_assume_slow(...) vsm_platform_assume(__VA_ARGS__)
#	define vsm_verify_slow(...) ((void)(__VA_ARGS__))
#endif

#if __cplusplus
} // extern "C"
#endif
