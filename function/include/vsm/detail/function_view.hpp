#pragma once

#include <vsm/detail/function_ptr.hpp>
#include <vsm/platform.h>
#include <vsm/utility.hpp>

namespace vsm::detail {

struct function_unused {};

union function_context
{
	function_unused unused;
	void const* object;
	function_ptr_t function;
};


struct get_function_context_cpo {};
//TODO: Use the get_function_context_cpo to optimise function_view.


template<bool N, typename T, typename R, typename... Ps>
vsm_detail_function_ptr_constexpr R invoke_object(function_context const context, Ps... args) noexcept(N)
{
	return const_cast<T&>(*static_cast<T const*>(context.object))(vsm_move(args)...);
}

template<bool N, typename F, typename R, typename... Ps>
vsm_detail_function_ptr_constexpr R invoke_function(function_context const context, Ps... args) noexcept(N)
{
	return static_cast<F*>(context.function)(vsm_move(args)...);
}

template<bool N, auto F, typename R, typename... Ps>
vsm_detail_function_ptr_constexpr R invoke_nontype_unused(function_context const context, Ps... args) noexcept(N)
{
	return F(vsm_move(args)...);
}

template<bool N, auto F, typename T, typename R, typename... Ps>
vsm_detail_function_ptr_constexpr R invoke_nontype_object(function_context const context, Ps... args) noexcept(N)
{
	return F(const_cast<T&>(*static_cast<T const*>(context.object)), vsm_move(args)...);
}

} // namespace vsm::detail
