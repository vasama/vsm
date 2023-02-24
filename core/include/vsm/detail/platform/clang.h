#include <vsm/detail/platform/gnu.h>

#define vsm_platform_assume(...) __builtin_assume(__VA_ARGS__)
