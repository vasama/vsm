#include <vsm/detail/platform/gnu.h>

#ifdef _MSC_VER
#	include <vsm/detail/platform/microsoft.h>
#endif

#define vsm_make_clang_version(major, minor, patch) \
	((major) << 16 | (minor) << 8 | (patch))

#define vsm_clang_version \
	vsm_make_clang_version(__clang_major__, __clang_minor__, __clang_patchlevel__)

#define vsm_gnu_diagnostic(...) vsm_detail_gnu_diagnostic(clang, __VA_ARGS__)
#define vsm_clang_diagnostic(...) vsm_detail_gnu_diagnostic(clang, __VA_ARGS__)

#define vsm_platform_assume(...) __builtin_assume(__VA_ARGS__)

#ifdef __CLANGD__
#	define vsm_compiler_preview 1
#endif

#ifdef __clang_tidy__
#	define vsm_static_analyzer 1
#endif
