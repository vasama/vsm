#pragma once

#include <vsm/concepts.hpp>
#include <vsm/lift.hpp>
#include <vsm/utility.hpp>

namespace vsm {
namespace detail {

template<typename Lambda>
class lazy
{
	using result_type = std::invoke_result_t<Lambda&&>;

	Lambda m_lambda;

public:
	explicit lazy(std::convertible_to<Lambda> auto&& lambda)
		: m_lambda(vsm_forward(lambda))
	{
	}


	operator result_type() &&
		noexcept(noexcept(vsm_move(m_lambda)()))
	{
		return vsm_move(m_lambda)();
	}

	template<typename T>
		requires std::is_convertible_v<result_type, T>
	operator T() &&
		noexcept(noexcept(T(vsm_move(m_lambda)())))
	{
		return vsm_move(m_lambda)();
	}

	template<typename T>
		requires std::is_reference_v<result_type> && std::is_convertible_v<result_type, T&>
	operator T&() &&
		noexcept(noexcept(vsm_move(m_lambda)()))
	{
		return vsm_move(m_lambda)();
	}

	template<typename T>
		requires std::is_reference_v<result_type> && std::is_convertible_v<result_type, T&&>
	operator T&&() &&
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
