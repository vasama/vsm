#pragma once

namespace vsm {

template<typename T>
class box
{
	T m_value;

public:
	template<typename... Args>
	constexpr box(Args&&... args)
		: m_value(static_cast<Args&&>(args)...)
	{
	}

	constexpr T& operator*() &
	{
		return m_value;
	}

	constexpr T&& operator*() &&
	{
		return static_cast<T&&>(m_value);
	}

	constexpr T const& operator*() const&
	{
		return m_value;
	}

	constexpr T const&& operator*() const&&
	{
		return static_cast<T const&&>(m_value);
	}

	constexpr T* operator->()
	{
		return &m_value;
	}

	constexpr T const* operator->() const
	{
		return &m_value;
	}
};

} // namespace vsm
