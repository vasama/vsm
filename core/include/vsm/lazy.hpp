#pragma once

#include <vsm/concepts.hpp>
#include <vsm/utility.hpp>

namespace vsm {
namespace detail {

template<typename Lambda>
class lazy
{
	Lambda m_lambda;

public:
	explicit lazy(std::convertible_to<Lambda> auto&& lambda)
		: m_lambda(vsm_forward(lambda))
	{
	}


	operator std::invoke_result_t<Lambda&&>() &&
		noexcept(noexcept(vsm_move(m_lambda)()))
	{
		return vsm_move(m_lambda)();
	}
};

template<typename Lambda>
lazy(Lambda) -> lazy<std::decay_t<Lambda>>;

} // namespace detail

#define vsm_lazy(...) (::vsm::detail::lazy( \
		[&]() \
			noexcept(noexcept(__VA_ARGS__)) \
			-> decltype(__VA_ARGS__) \
		{ return __VA_ARGS__; } \
	))

} // namespace vsm
