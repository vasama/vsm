#include <vsm/detail/platform/gnu.h>

#ifdef _MSC_VER
#	include <vsm/detail/platform/microsoft.h>
#endif

#define vsm_gnu_diagnostic(...) vsm_detail_gnu_diagnostic(clang, __VA_ARGS__)
#define vsm_clang_diagnostic(...) vsm_detail_gnu_diagnostic(clang, __VA_ARGS__)

#define vsm_platform_assume(...) __builtin_assume(__VA_ARGS__)
