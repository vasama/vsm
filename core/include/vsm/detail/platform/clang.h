#include <vsm/detail/platform/gnu.h>

#define vsm_gnu_diagnostic(...) vsm_detail_gnu_diagnostic(clang, __VA_ARGS__)
#define vsm_clang_diagnostic(...) vsm_detail_gnu_diagnostic(clang, __VA_ARGS__)

#define vsm_platform_assume(...) __builtin_assume(__VA_ARGS__)
