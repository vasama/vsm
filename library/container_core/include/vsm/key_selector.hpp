#pragma once

#include <vsm/array_view.hpp>
#include <vsm/concepts.hpp>
#include <vsm/standard.hpp>
#include <vsm/tag_invoke.hpp>

#include <tuple>
#include <string_view>

namespace vsm {
namespace detail {

template<typename T, size_t I>
concept tuple_like_2 = requires (T& tuple)
{
	typename std::tuple_element<I, T>::type;
	{ get<I>(tuple) } -> vsm::any_ref_of<typename std::tuple_element<I, T>::type>;
};

template<typename T, size_t... Is>
void tuple_like_1(std::index_sequence<Is...>)
	requires (tuple_like_2<T, Is> && ...);

template<typename T>
concept tuple_like = requires
{
	{ std::tuple_size<T>::value } -> std::integral;
	detail::tuple_like_1<T>(std::make_index_sequence<std::tuple_size<T>::value>());
};


template<typename T, size_t... Is>
auto make_key_tuple(T const& tuple, std::index_sequence<Is...>)
{
	return std::tuple<std::tuple_element_t<Is, T> const&...>(get<Is>(tuple)...);
}


struct select_key_cpo
{
	template<typename T, typename Selector>
	friend T const& tag_invoke(select_key_cpo, T const& key, Selector const&)
	{
		return key;
	}

	template<typename T, typename Selector>
	[[nodiscard]] vsm_static_operator decltype(auto) operator()(
		T const& key,
		Selector const& selector) vsm_static_operator_const noexcept
	{
		return tag_invoke(select_key_cpo(), key, selector);
	}
};

struct normalize_key_cpo
{
	template<typename... Ts>
	friend std::tuple<Ts...> const& tag_invoke(normalize_key_cpo, std::tuple<Ts...> const& tuple)
	{
		return tuple;
	}

	template<tuple_like T>
	friend auto tag_invoke(normalize_key_cpo, T const& tuple)
	{
		return detail::make_key_tuple(tuple, std::make_index_sequence<std::tuple_size_v<T>>());
	}


	template<typename T, size_t Size>
	friend array_view<T> tag_invoke(normalize_key_cpo, T const(&array)[Size])
	{
		return array_view<T>(array);
	}

	template<character T, size_t Size>
	friend std::basic_string_view<T> tag_invoke(normalize_key_cpo, T const(&array)[Size])
	{
		return std::basic_string_view<T>(array);
	}


	template<typename T>
	friend T const& tag_invoke(normalize_key_cpo, T const& key)
	{
		return key;
	}

	template<typename T>
	[[nodiscard]] vsm_static_operator decltype(auto) operator()(T const& key) vsm_static_operator_const noexcept
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
	[[nodiscard]] vsm_static_operator T const& operator()(
		T const& key) vsm_static_operator_const noexcept
	{
		return key;
	}
};

struct address_key_selector
{
	template<typename T>
	[[nodiscard]] vsm_static_operator T const* operator()(
		T const& key) vsm_static_operator_const noexcept
	{
		return &key;
	}
};

struct default_key_selector
{
	template<typename T>
	[[nodiscard]] vsm_static_operator decltype(auto) operator()(
		T const& key) vsm_static_operator_const noexcept
	{
		return tag_invoke(select_key, key, default_key_selector());
	}
};

} // namespace vsm
