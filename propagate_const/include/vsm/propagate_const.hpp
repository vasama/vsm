#pragma once

#include <vsm/concepts.hpp>
#include <vsm/utility.hpp>

#include <memory>
#include <span>

namespace vsm {
namespace detail {

template<typename T>
concept propagate_const_arrow =
	std::is_pointer_v<std::remove_cvref_t<T>> ||
	requires (T const& p) { p.operator->(); };

template<typename P>
struct propagate_const_traits
{
	using get_type = typename std::pointer_traits<P>::element_type*;
	using get_const_type = typename std::pointer_traits<P>::element_type const*;
	using difference_type = typename std::pointer_traits<P>::difference_type;

	static get_type get(P const& ptr)
	{
		return std::to_address(ptr);
	}

	static get_const_type get_const(P const& ptr)
	{
		return std::to_address(ptr);
	}
};

template<typename T>
struct propagate_const_traits<std::span<T>>
{
	using get_type = std::span<T>;
	using get_const_type = std::span<T const>;
	using difference_type = typename std::span<T>::difference_type;

	static get_type get(std::span<T> const& span)
	{
		return span;
	}
	
	static get_const_type get_const(std::span<T> const& span)
	{
		return span;
	}
};

} // namespace detail

using detail::propagate_const_traits;

template<typename Underlying>
class propagate_const
{
	using underlying_type = std::remove_cv_t<Underlying>;
	using traits = detail::propagate_const_traits<underlying_type>;

	using get_type = typename traits::get_type;
	using get_const_type = typename traits::get_const_type;
	using difference_type = typename traits::difference_type;

	Underlying m_value;

public:
	propagate_const() = default;

	template<no_cvref_of<propagate_const> T = underlying_type>
	explicit(!std::is_convertible_v<T&&, Underlying>)
	constexpr propagate_const(T&& value)
		requires std::constructible_from<Underlying, T&&>
		: m_value(vsm_forward(value))
	{
	}

	propagate_const(propagate_const&&) = default;

	constexpr propagate_const(propagate_const& other)
		: m_value(vsm_as_const(other.m_value))
	{
	}

	propagate_const(propagate_const const& other) = delete;

	template<no_cvref_of<propagate_const> T = underlying_type>
	constexpr propagate_const& operator=(T&& value) &
		requires std::assignable_from<Underlying&, T&&>
	{
		m_value = vsm_forward(value);
		return *this;
	}

	propagate_const& operator=(propagate_const&&) & = default;

	constexpr propagate_const& operator=(propagate_const& other) &
	{
		m_value = vsm_as_const(other.m_value);
		return *this;
	}

	propagate_const& operator=(propagate_const const& other) & = delete;


	constexpr Underlying& underlying() &
	{
		return m_value;
	}

	constexpr Underlying&& underlying() &&
	{
		return vsm_move(m_value);
	}


	constexpr get_type get()
	{
		return traits::get(vsm_as_const(m_value));
	}

	constexpr get_const_type get() const
	{
		return traits::get_const(vsm_as_const(m_value));
	}


	constexpr decltype(auto) operator*()
		requires requires (underlying_type const& p) { *p; }
	{
		return *get();
	}

	constexpr decltype(auto) operator*() const
		requires requires (underlying_type const& p) { *p; }
	{
		return *get();
	}

	constexpr decltype(auto) operator->()
		requires detail::propagate_const_arrow<underlying_type const&>
	{
		return get();
	}

	constexpr decltype(auto) operator->() const
		requires detail::propagate_const_arrow<underlying_type const&>
	{
		return get();
	}

	constexpr decltype(auto) operator[](difference_type const& index)
		requires requires (underlying_type const& p) { p[index]; }
	{
		return get()[index];
	}

	constexpr decltype(auto) operator[](difference_type const& index) const
		requires requires (underlying_type const& p) { p[index]; }
	{
		return get()[index];
	}


	friend auto operator<=>(propagate_const const&, propagate_const const&) = default;

	friend constexpr bool operator==(propagate_const const& p, auto const& other)
		requires requires { p.m_value == other; }
	{
		return p.m_value == other;
	}

	friend constexpr bool operator<=>(propagate_const const& p, auto const& other)
		requires requires { p.m_value <=> other; }
	{
		return p.m_value <=> other;
	}
};

} // namespace vsm
