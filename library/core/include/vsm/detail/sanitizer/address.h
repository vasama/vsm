#pragma once

#include <vsm/platform.h>

#include <sanitizer/asan_interface.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#if vsm_compiler_msvc
#	ifdef __INTELLISENSE__
#		define vsm_no_sanitize_address
#	else
#		define vsm_no_sanitize_address __declspec(no_sanitize_address)
#	endif
#else
#	define vsm_no_sanitize_address __attribute__((no_sanitize_address))
#endif

vsm_no_sanitize_address
void* vsm_memcpy_no_sanitize_address(void* dst, void const* src, size_t size);

vsm_no_sanitize_address
void* vsm_memset_no_sanitize_address(void* dst, int value, size_t size);

vsm_no_sanitize_address
void* vsm_memmove_no_sanitize_address(void* dst, void const* src, size_t size);

#ifdef __cplusplus
} // extern "C"
#endif
