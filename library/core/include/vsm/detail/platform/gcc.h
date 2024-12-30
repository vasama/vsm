#include <vsm/detail/platform/gnu.h>

#define vsm_gnu_diagnostic(...) vsm_detail_gnu_diagnostic(GCC, __VA_ARGS__)
#define vsm_gcc_diagnostic(...) vsm_detail_gnu_diagnostic(GCC, __VA_ARGS__)
