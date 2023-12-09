#define vsm_detail_gnu_diagnostic(compiler, ...) \
	_Pragma(vsm_pp_str(compiler diagnostic __VA_ARGS__))

#define vsm_always_inline __attribute__((always_inline))
#define vsm_never_inline __attribute__((noinline))

#define vsm_likely(...) __builtin_expect((__VA_ARGS__), 1)
#define vsm_unlikely(...) __builtin_expect((__VA_ARGS__), 0)

#define vsm_unreachable() __builtin_unreachable()

#define vsm_alloca(...) __builtin_alloca(__VA_ARGS__)
