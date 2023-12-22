#ifndef vsm_compiler_clang
#	define vsm_compiler_clang 0
#endif

#ifndef vsm_compiler_msvc
#	define vsm_compiler_msvc 0
#endif

#ifndef vsm_compiler_gcc
#	define vsm_compiler_gcc 0
#endif

#ifndef vsm_gnu_diagnostic
#	define vsm_gnu_diagnostic(...)
#endif

#ifndef vsm_clang_diagnostic
#	define vsm_clang_diagnostic(...)
#endif

#ifndef vsm_gcc_diagnostic
#	define vsm_gcc_diagnostic(...)
#endif

#ifndef vsm_msvc_warning
#	define vsm_msvc_warning(...)
#endif

#ifndef vsm_likely
#	define vsm_likely(...) (__VA_ARGS__)
#endif

#ifndef vsm_unlikely
#	define vsm_unlikely(...) (__VA_ARGS__)
#endif

#ifndef vsm_platform_assume
#	define vsm_platform_assume(...) ((void)0)
#endif

#ifndef vsm_unreachable
#	define vsm_unreachable() ((void)0)
#endif

#ifndef vsm_restrict
#	define vsm_restrict __restrict
#endif
