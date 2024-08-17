#pragma once

#include <vsm/concepts.hpp>
#include <vsm/utility.hpp>

#include <memory>
#include <span>

namespace vsm {
namespace detail {

template<typename T>
concept _prop_cv_to_address = requires (T const& p)
{
	std::to_address(p);
};

template<typename P>
concept _prop_cv_ptr = requires
{
	typename std::pointer_traits<P>::pointer;
	typename std::pointer_traits<P>::element_type;
	typename std::pointer_traits<P>::difference_type;
	typename std::pointer_traits<P>::template rebind<typename std::pointer_traits<P>::element_type>;
};

template<typename P>
using _prop_cv_rebind_const =
	typename std::pointer_traits<P>::template rebind<
		typename std::pointer_traits<P>::element_type const>;

template<typename P, typename ConstP>
concept _prop_cv_convertible_ptr =
	std::is_copy_constructible_v<P> &&
	std::is_copy_constructible_v<ConstP> &&
	std::is_convertible_v<ConstP, P>;

template<typename P>
concept _prop_cv_const_convertible_ptr =
	_prop_cv_ptr<P> &&
	_prop_cv_convertible_ptr<P, _prop_cv_rebind_const<P>>;


template<typename P>
struct _prop_cv_traits
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

template<_prop_cv_const_convertible_ptr P>
struct _prop_cv_traits<P>
{
	using get_type = P;
	using get_const_type = _prop_cv_rebind_const<P>;
	using difference_type = typename std::pointer_traits<P>::difference_type;

	static_assert(std::is_nothrow_copy_constructible_v<get_type>);
	static_assert(std::is_nothrow_copy_constructible_v<get_const_type>);
	static_assert(std::is_nothrow_convertible_v<get_type, get_const_type>);

	static get_type get(P const& ptr)
	{
		return ptr;
	}

	static get_const_type get_const(P const& ptr)
	{
		return ptr;
	}
};

template<typename P>
struct prop_cv_traits : _prop_cv_traits<P> {};

template<typename T>
struct prop_cv_traits<std::span<T>>
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

using detail::prop_cv_traits;

template<typename Underlying>
class prop_cv
{
	using underlying_type = std::remove_cv_t<Underlying>;
	using traits = detail::prop_cv_traits<underlying_type>;

	using get_type = typename traits::get_type;
	using get_const_type = typename traits::get_const_type;
	using difference_type = typename traits::difference_type;

	Underlying m_value;

public:
	prop_cv() = default;

	template<no_cvref_of<prop_cv> T = underlying_type>
	explicit(!std::is_convertible_v<T&&, Underlying>)
	constexpr prop_cv(T&& value)
		requires std::constructible_from<Underlying, T&&>
		: m_value(vsm_forward(value))
	{
	}

	prop_cv(prop_cv&&) = default;

	constexpr prop_cv(prop_cv& other)
		: m_value(vsm_as_const(other.m_value))
	{
	}

	prop_cv(prop_cv const& other) = delete;

	template<no_cvref_of<prop_cv> T = underlying_type>
	constexpr prop_cv& operator=(T&& value) &
		requires std::assignable_from<Underlying&, T&&>
	{
		m_value = vsm_forward(value);
		return *this;
	}

	prop_cv& operator=(prop_cv&&) & = default;

	constexpr prop_cv& operator=(prop_cv& other) &
	{
		m_value = vsm_as_const(other.m_value);
		return *this;
	}

	prop_cv& operator=(prop_cv const& other) & = delete;


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
		requires detail::_prop_cv_to_address<underlying_type const&>
	{
		return get();
	}

	constexpr decltype(auto) operator->() const
		requires detail::_prop_cv_to_address<underlying_type const&>
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


	friend auto operator<=>(prop_cv const&, prop_cv const&) = default;

	friend constexpr bool operator==(prop_cv const& p, auto const& other)
		requires requires { p.m_value == other; }
	{
		return p.m_value == other;
	}

	friend constexpr bool operator<=>(prop_cv const& p, auto const& other)
		requires requires { p.m_value <=> other; }
	{
		return p.m_value <=> other;
	}
};

} // namespace vsm
