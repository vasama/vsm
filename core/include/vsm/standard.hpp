#pragma once

#if __cpp_if_consteval
#	define vsm_if_consteval \
		if consteval
#else
#	include <type_traits>

#	define vsm_if_consteval \
		if (::std::is_constant_evaluated())
#endif

#ifdef __cpp_static_call_operator
#	define vsm_static_operator static
#	define vsm_static_operator_const
#else
#	define vsm_static_operator
#	define vsm_static_operator_const const
#endif
