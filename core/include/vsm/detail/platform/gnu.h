#define vsm_always_inline __attribute__((always_inline))
#define vsm_never_inline __attribute__((noinline))

#define vsm_likely(...) __builtin_expect((__VA_ARGS__), 1)
#define vsm_unlikely(...) __builtin_expect((__VA_ARGS__), 0)

#define vsm_unreachable() __builtin_unreachable()
