#if __has_include(<x86intrin.h>)
#	include <x86intrin.h>
#elif __has_include(<intrin.h>)
#	include <intrin.h>
#else
#	error x86 intrinsics header not found
#endif
