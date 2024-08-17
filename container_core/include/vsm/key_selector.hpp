#pragma once

#include <vsm/concepts.hpp>
#include <vsm/standard.hpp>
#include <vsm/tag_invoke.hpp>

namespace vsm {
namespace detail {

struct select_key_cpo
{
	template<typename T, typename Selector>
	friend T const& tag_invoke(select_key_cpo, T const& key, Selector const&)
	{
		return key;
	}

	template<typename T, typename Selector>
	vsm_static_operator decltype(auto) operator()(
		T const& key,
		Selector const& selector) vsm_static_operator_const noexcept
	{
		return tag_invoke(select_key_cpo(), key, selector);
	}
};

struct normalize_key_cpo
{
	template<typename T>
	friend T const& tag_invoke(normalize_key_cpo, T const& key)
	{
		return key;
	}

	template<typename T>
	vsm_static_operator decltype(auto) operator()(T const& key) vsm_static_operator_const noexcept
		requires tag_invocable<normalize_key_cpo, T const&>
	{
		return tag_invoke(normalize_key_cpo(), key);
	}
};

} // namespace detail

inline constexpr detail::select_key_cpo select_key = {};
inline constexpr detail::normalize_key_cpo normalize_key = {};


template<typename KeySelector, typename T>
concept key_selector = requires (KeySelector const& key_selector, T const& object)
{
	{ key_selector(object) } -> not_same_as<void>;
};

struct identity_key_selector
{
	template<typename T>
	vsm_static_operator T const& operator()(T const& key) vsm_static_operator_const noexcept
	{
		return key;
	}
};

struct address_key_selector
{
	template<typename T>
	vsm_static_operator T const* operator()(T const& key) vsm_static_operator_const noexcept
	{
		return &key;
	}
};

struct default_key_selector
{
	template<typename T>
	vsm_static_operator decltype(auto) operator()(T const& key) vsm_static_operator_const noexcept
	{
		return tag_invoke(select_key, key, default_key_selector());
	}
};

} // namespace vsm
