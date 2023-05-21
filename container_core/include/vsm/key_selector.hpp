#pragma once

#include <vsm/concepts.hpp>
#include <vsm/standard.hpp>

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
	vsm_static_operator decltype(auto) operator()(T const& key, Selector const& selector) vsm_static_operator_const noexcept
	{
		return tag_invoke(*this, key, selector);
	}
};

struct simplify_key_cpo
{
	template<typename T, typename Selector>
	friend T const& tag_invoke(simplify_key_cpo, T const& key, Selector const&)
	{
		return key;
	}

	template<typename T, typename Selector>
	vsm_static_operator decltype(auto) operator()(T const& key, Selector const& selector) vsm_static_operator_const noexcept
	{
		return tag_invoke(*this, key, selector);
	}
};

} // namespace detail

inline constexpr detail::select_key_cpo select_key = {};
inline constexpr detail::simplify_key_cpo simplify_key = {};


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
		decltype(auto) k = tag_invoke(select_key, key, *this);
		decltype(auto) s = tag_invoke(simplify_key, k, *this);

		if constexpr (std::is_reference_v<decltype(k)>)
		{
			return s;
		}
		else
		{
			return std::remove_reference_t<decltype(s)>(s);
		}
	}
};

} // namespace vsm
