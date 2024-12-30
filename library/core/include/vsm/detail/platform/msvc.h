#include <vsm/detail/platform/microsoft.h>

// NOLINTBEGIN(modernize-macro-to-enum)

#define vsm_msvc_warning(...) __pragma(warning(__VA_ARGS__))

#define vsm_always_inline __forceinline
#define vsm_never_inline __declspec(noinline)

#define vsm_analysis_assume __assume
#define vsm_optimize_assume __assume

#define vsm_unreachable() __assume(0)

#define vsm_alloca(...) _alloca(__VA_ARGS__)

#ifdef __INTELLISENSE__
#	define vsm_compiler_preview 1
#endif

#ifdef _PREFAST_
#	define vsm_static_analyzer 1
#endif

// NOLINTEND(modernize-macro-to-enum)
