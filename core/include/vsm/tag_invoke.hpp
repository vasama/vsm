#pragma once

#include <vsm/platform.h>
#include <vsm/standard.hpp>
#include <vsm/utility.hpp>

namespace vsm {
namespace detail {
namespace _tag_invoke_t {

void tag_invoke();

template<typename T, typename... Args>
concept tag_invocable = requires(T&& tag, Args&&... args)
{
	tag_invoke(static_cast<T&&>(tag), static_cast<Args&&>(args)...);
};

template<typename T, typename... Args>
concept nothrow_tag_invocable = tag_invocable<T, Args...> && requires(T&& tag, Args&&... args)
{
	{ tag_invoke(static_cast<T&&>(tag), static_cast<Args&&>(args)...) } noexcept;
};

template<typename T, typename... Args>
using tag_invoke_result_t = decltype(tag_invoke(std::declval<T>(), std::declval<Args>()...));

struct tag_invoke_t
{
	template<typename T, typename... Args>
		requires tag_invocable<T, Args...>
	constexpr vsm_always_inline auto vsm_static_operator_invoke(T tag, Args&&... args)
		noexcept(nothrow_tag_invocable<T, Args...>)
		-> tag_invoke_result_t<T, Args...>
	{
		return tag_invoke(static_cast<T&&>(tag), static_cast<Args&&>(args)...);
	}
};

} // namespace _tag_invoke_t
namespace _tag_invoke {

inline constexpr _tag_invoke_t::tag_invoke_t tag_invoke = {};

} // namespace _tag_invoke
} // namespace detail

using detail::_tag_invoke_t::tag_invocable;
using detail::_tag_invoke_t::nothrow_tag_invocable;
using detail::_tag_invoke_t::tag_invoke_result_t;
using detail::_tag_invoke_t::tag_invoke_t;
using namespace detail::_tag_invoke;


template<typename T>
struct tag
{
	using type = T;
};

} // namespace vsm
