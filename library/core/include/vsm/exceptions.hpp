#pragma once

#ifndef vsm_config_exceptions
#	include <vsm/platform.h>

#	if vsm_stdlib_msstl
#		if _HAS_EXCEPTIONS
#			define vsm_config_exceptions 1
#		else
#			define vsm_config_exceptions 0
#		endif
#	elif !__cpp_exceptions
#		define vsm_config_exceptions 0
#	else
#		define vsm_config_exceptions 1
#	endif
#endif

#include <exception>

#if vsm_config_exceptions

#	define vsm_except_try \
	try

#	define vsm_except_catch(...) \
	catch (__VA_ARGS__)

#	define vsm_except_throw(...) \
	(throw (__VA_ARGS__))

#	define vsm_except_throw_or_terminate(...) \
	(throw __VA_ARGS__)

#	define vsm_except_rethrow \
	throw

#else

#	define vsm_except_try \
	if constexpr (1)

#	define vsm_except_catch(...) \
	else if constexpr (0)

#	define vsm_except_throw(...) \
	((void)0)

#	define vsm_except_throw_or_terminate(...) \
	(::std::terminate())

#	define vsm_except_rethrow \
	((void)0)

#endif
