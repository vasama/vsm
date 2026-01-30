#pragma once

#if __SANITIZE_ADDRESS__
#	define vsm_has_address_sanitizer 1
#else
#	ifdef __has_feature
#		if __has_feature(address_sanitizer)
#			define vsm_has_address_sanitizer 1
#		else
#			define vsm_has_address_sanitizer 0
#		endif
#	else
#		define vsm_has_address_sanitizer 0
#	endif
#endif

#if vsm_has_address_sanitizer
#	include <vsm/detail/sanitizer/address.h>
#else
#	define vsm_no_sanitize_address
#	define vsm_memcpy_no_sanitize_address memcpy
#	define vsm_memset_no_sanitize_address memset
#	define vsm_memmove_no_sanitize_address memmove
#endif
