#pragma once

#include <type_traits>

/// @brief Macro implementation of std::forward.
/// Does not require specifying forwarded object type.
#define vsm_forward(...) \
	static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

/// @brief Macro implementation of std::move.
#define vsm_move(...) \
	static_cast<::std::remove_reference_t<decltype(__VA_ARGS__)>&&>(__VA_ARGS__)

/// @brief Macro implementation of std::as_const.
#define vsm_as_const(...) \
	const_cast<::std::remove_reference_t<decltype(__VA_ARGS__)> const&>(__VA_ARGS__)

/// @brief Macro implementation of std::declval.
#define vsm_declval(...) \
	(static_cast<::std::add_rvalue_reference_t<__VA_ARGS__>(*)()>(0)())
