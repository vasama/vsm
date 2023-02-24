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
