#pragma once

#include <vsm/utility.hpp>

namespace vsm {
namespace detail::tag_invoke_ {

void tag_invoke() = delete;

struct cpo
{
	constexpr decltype(auto) operator()(auto const tag, auto&&... args) const
		noexcept(noexcept(tag_invoke(tag, vsm_forward(args)...)))
		requires (requires { tag_invoke(tag, vsm_forward(args)...); })
	{
		return tag_invoke(vsm_forward(tag), vsm_forward(args)...);
	}
};

} // namespace detail::tag_invoke_

template<typename>
struct tag {};

inline constexpr detail::tag_invoke_::cpo tag_invoke = {};

template<typename T, typename... Args>
concept tag_invocable = requires(T&& tag, Args&&... args)
{
	tag_invoke(vsm_forward(tag), vsm_forward(args)...);
};

} // namespace vsm
