#pragma once

#include <concepts>
#include <utility>

namespace vsm {

template<typename T>
class arrow
{
	T m_value;

public:
	template<std::convertible_to<T> U = T>
	constexpr arrow(U&& value)
		: m_value(static_cast<U&&>(value))
	{
	}

	template<typename... Args>
		requires std::constructible_from<T, Args...>
	explicit constexpr arrow(std::in_place_t, Args&&... args)
		: m_value(static_cast<Args&&>(args)...)
	{
	}

	constexpr T* operator->()
	{
		return std::addressof(m_value);
	}

	constexpr T const* operator->() const
	{
		return std::addressof(m_value);
	}
};

} // namespace vsm
