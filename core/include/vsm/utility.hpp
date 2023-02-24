#pragma once

#include <type_traits>

#define vsm_forward(...) \
	static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

#define vsm_move(...) \
	static_cast<::std::remove_reference_t<decltype(__VA_ARGS__)>&&>(__VA_ARGS__)

#define vsm_as_const(...) \
	const_cast<::std::remove_reference_t<decltype(__VA_ARGS__)> const&>(__VA_ARGS__)
