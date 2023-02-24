#define vsm_msvc_warning(...) __pragma(warning(__VA_ARGS__))

#define vsm_always_inline __forceinline
#define vsm_never_inline __declspec(noinline)

#define vsm_platform_assume(...) __assume(__VA_ARGS__)

#define vsm_unreachable() __assume(0)
