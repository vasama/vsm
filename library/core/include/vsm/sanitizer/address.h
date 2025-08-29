#pragma once

#if __has_include(<sanitizer/asan_interface.h>)
#	include <sanitizer/asan_interface.h>

#	if defined(__has_feature)
#		if __has_feature(address_sanitizer)
#			define vsm_has_address_sanitizer 1
#		else
#			define vsm_has_address_sanitizer 0
#		endif
#	else
#		define vsm_has_address_sanitizer 0
#	endif
#else
#	define vsm_has_address_sanitizer 0
#endif
