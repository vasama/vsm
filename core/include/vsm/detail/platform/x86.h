#define vsm_detail_platform_x86_intrin

#if __has_include(<x86intrin.h>)
#	include <x86intrin.h>
#elif __has_include(<intrin.h>)
#	include <intrin.h>
#else
#	warning No x86 intrinsics header.
#	undef vsm_detail_platform_x86_intrin
#endif
