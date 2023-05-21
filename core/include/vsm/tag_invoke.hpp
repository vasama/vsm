#pragma once

#include <vsm/standard.hpp>
#include <vsm/utility.hpp>

namespace vsm {
namespace detail {
namespace tag_invoke_1 {

void tag_invoke();

struct cpo
{
	vsm_static_operator constexpr decltype(auto)
		operator()(auto const tag, auto&&... args) vsm_static_operator_const
		requires requires { tag_invoke(tag, vsm_forward(args)...); }
	{
		return tag_invoke(tag, vsm_forward(args)...);
	}
};

} // namespace tag_invoke_1
namespace tag_invoke_2 {

inline constexpr tag_invoke_1::cpo tag_invoke = {};

} // namespace tag_invoke_2
} // namespace detail

using namespace detail::tag_invoke_2;

template<typename>
struct tag {};

template<typename T, typename... Args>
concept tag_invocable = requires(T const& tag, Args&&... args)
{
	tag_invoke(tag, vsm_forward(args)...);
};

} // namespace vsm
