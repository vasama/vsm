#pragma once

#include <vsm/platform.h>

#include <memory>

/* C++20 std::destroying_delete_t */

#if vsm_compiler_msvc
#	include <vsm/detail/msvc_delete.hpp>
#else
#	define vsm_qualified_delete(p) (::delete(p))
#endif


/* C++20 [[no_unique_address]] */

#if __has_cpp_attribute(msvc::no_unique_address)
#	define vsm_no_unique_address [[msvc::no_unique_address]]
#elif __has_cpp_attribute(no_unique_address)
#	define vsm_no_unique_address [[no_unique_address]]
#else
#	define vsm_no_unique_address
#endif


/* C++23 lambda attributes */

#if vsm_compiler_msvc
#	define vsm_lambda_attribute(...)
#else
#	define vsm_lambda_attribute(...) __VA_ARGS__
#endif

#define vsm_lambda_nodiscard vsm_lambda_attribute( [[nodiscard]] )


/* C++23 if consteval */

#if __cpp_if_consteval
#	define vsm_if_consteval if consteval
#else
#	include <type_traits>
#	define vsm_if_consteval if (::std::is_constant_evaluated())
#endif


/* C++23 static operator() and operator[] */

#if __cpp_static_call_operator
#	define vsm_static_operator static
#	define vsm_static_operator_const
#else
#	define vsm_static_operator
#	define vsm_static_operator_const const
#endif


/* C++23 auto cast */

#if __cpp_auto_cast
#	define vsm_decay_copy(...) auto(__VA_ARGS__)
#else
#	include <type_traits>
#	define vsm_decay_copy(...) static_cast<::std::decay_t<decltype(__VA_ARGS__)>>(__VA_ARGS__)
#endif


/* C++23 explicit lifetime management */

namespace vsm {

#if __cpp_lib_start_lifetime_as
	using std::start_lifetime_as;
	using std::start_lifetime_as_array;
#else
#	include <vsm/detail/start_lifetime_as.ipp>
#endif

} // namespace vsm
