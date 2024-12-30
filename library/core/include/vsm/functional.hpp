#pragma once

#include <vsm/concepts.hpp>
#include <vsm/utility.hpp>

#include <functional>

namespace vsm {

struct addressof_t
{
	template<typename T>
	constexpr std::remove_reference_t<T>* operator()(T&& reference) const noexcept
	{
		return std::addressof(reference);
	}
};
inline constexpr addressof_t addressof;

struct ampersand_t
{
	template<typename T>
	constexpr decltype(auto) operator()(T&& reference) const noexcept
	{
		return &vsm_forward(reference);
	}
};
inline constexpr ampersand_t ampersand;

template<typename T>
class equal_to
{
	T m_value;

public:
	template<no_cvref_of<equal_to> U>
		requires std::convertible_to<U, T>
	explicit equal_to(U&& value)
		: m_value(vsm_forward(value))
	{
	}

	template<std::equality_comparable_with<T> U>
	[[nodiscard]] bool operator()(U const& rhs) const
	{
		return m_value == rhs;
	}
};

template<typename T>
equal_to(T) -> equal_to<T>;

} // namespace vsm
