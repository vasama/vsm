#include <vsm/detail/platform/microsoft.h>

#define vsm_msvc_warning(...) __pragma(warning(__VA_ARGS__))

#define vsm_always_inline __forceinline
#define vsm_never_inline __declspec(noinline)

#define vsm_platform_assume(...) __assume(__VA_ARGS__)

#define vsm_unreachable() __assume(0)

#define vsm_alloca(...) _alloca(__VA_ARGS__)

#ifdef __INTELLISENSE__
#	define vsm_compiler_preview 1
#endif
