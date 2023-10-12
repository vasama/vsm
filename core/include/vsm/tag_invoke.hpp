#pragma once

#include <vsm/standard.hpp>
#include <vsm/utility.hpp>

namespace vsm {
namespace detail {
namespace _tag_invoke_t {

void tag_invoke();

struct tag_invoke_t
{
	template<typename Tag, typename... Args>
		requires requires { tag_invoke(std::declval<Tag>(), std::declval<Args>()...); }
	vsm_static_operator constexpr decltype(auto)
		operator()(Tag, Args&&... args) vsm_static_operator_const
		noexcept(noexcept(tag_invoke(Tag(), vsm_forward(args)...)))
	{
		return tag_invoke(Tag(), vsm_forward(args)...);
	}
};

} // namespace _tag_invoke_t
namespace _tag_invoke {

inline constexpr _tag_invoke_t::tag_invoke_t tag_invoke = {};

} // namespace _tag_invoke
} // namespace detail

using namespace detail::_tag_invoke;

template<typename T, typename... Args>
concept tag_invocable = requires(T const& tag, Args&&... args)
{
	tag_invoke(tag, vsm_forward(args)...);
};

template<typename T, typename... Args>
concept nothrow_tag_invocable = tag_invocable<T, Args...> && requires(T const& tag, Args&&... args)
{
	{ tag_invoke(tag, vsm_forward(args)...) } noexcept;
};

template<typename T, typename... Args>
using tag_invoke_result_t = decltype(tag_invoke(std::declval<T>(), std::declval<Args>()...));


template<typename T>
struct tag
{
	using type = T;
};

} // namespace vsm
