#pragma once

#include <utility>

#include <cstddef>

namespace vsm {

template<typename... Ts>
struct type_list
{
	static constexpr size_t size = sizeof...(Ts);
};

namespace detail {

template<typename, size_t>
struct indexed_type {};

template<typename Indices, typename... Ts>
struct indexed_type_list;

template<size_t... Indices, typename... Ts>
struct indexed_type_list<std::index_sequence<Indices...>, Ts...> : indexed_type<Ts, Indices>... {};


template<size_t Index, typename T>
T type_list_at_2(indexed_type<T, Index>);

template<size_t Index, typename... Ts>
auto type_list_at_1(type_list<Ts...>) -> decltype(type_list_at_2<Index>(indexed_type_list<std::make_index_sequence<sizeof...(Ts)>, Ts...>()));

template<typename List, size_t Index>
using type_list_at = decltype(type_list_at_1<Index>(List{}));


template<typename T, size_t Index>
std::integral_constant<size_t, Index> type_list_index_2(indexed_type<T, Index>);

template<typename T, typename... Ts>
auto type_list_index_1(type_list<Ts...>) -> decltype(type_list_index_2<T>(indexed_type_list<std::make_index_sequence<sizeof...(Ts)>, Ts...>()));

template<typename T, typename U>
auto type_list_index_1(U)
{
	static_assert(sizeof(U) == 0, "T is not present in the type_list U.");
	return std::integral_constant<size_t, 0>();
}

template<typename List, typename T>
inline constexpr size_t type_list_index = decltype(type_list_index_1<T>(List{}))::value;


template<typename... Lhs, typename... Rhs>
type_list<Lhs..., Rhs...> operator+(type_list<Lhs...>, type_list<Rhs...>);

template<typename... Lists>
using type_list_cat = decltype((Lists{} + ...));

} // namespace detail

using detail::type_list_at;
using detail::type_list_index;
using detail::type_list_cat;

} // namespace vsm
