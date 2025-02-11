#pragma once

#include <concepts>
#include <new>
#include <type_traits>

#include <cstddef>
#include <cstdint>

namespace vsm {
namespace detail::_type_traits {

/* type comparison */

template<bool>
struct all_same;

template<>
struct all_same<0>
{
	template<typename...>
	static constexpr bool value = true;
};

template<>
struct all_same<1>
{
	template<typename T, typename... Ts>
	static constexpr bool value = (std::is_same_v<T, Ts> && ...);
};


/* conditional */

template<bool>
struct select;

template<>
struct select<0>
{
	template<typename, typename T>
	using type_template = T;
};

template<>
struct select<1>
{
	template<typename T, typename>
	using type_template = T;
};


/* value category copy */

//TODO: Look into copy compile time performance
#if 1
template<typename T>
struct copy
{
	template<typename U>
	using cv = U;

	template<typename U>
	using ref = U;

	template<typename U>
	using cvref = U;
};

template<typename T>
struct copy<T const>
{
	template<typename U>
	using cv = U const;

	template<typename U>
	using ref = U;

	template<typename U>
	using cvref = U;
};

template<typename T>
struct copy<T&>
{
	template<typename U>
	using ref = U&;

	template<typename U>
	using cvref = U&;
};

template<typename T>
struct copy<T const&>
{
	template<typename U>
	using ref = U&;

	template<typename U>
	using cvref = U const&;
};

template<typename T>
struct copy<T&&>
{
	template<typename U>
	using ref = U&&;

	template<typename U>
	using cvref = U&&;
};

template<typename T>
struct copy<T const&&>
{
	template<typename U>
	using ref = U&&;

	template<typename U>
	using cvref = U const&&;
};
#else
struct _copy
{
	template<typename U>
	using cv = U;

	template<typename U>
	using ref = U;

	template<typename U>
	using cvref = U;
};

struct _copy_c
{
	template<typename U>
	using cv = U const;

	template<typename U>
	using ref = U;

	template<typename U>
	using cvref = U;
};

struct _copy_l
{
	template<typename U>
	using ref = U&;

	template<typename U>
	using cvref = U&;
};

struct _copy_cl
{
	template<typename U>
	using ref = U&;

	template<typename U>
	using cvref = U const&;
};

struct _copy_r
{
	template<typename U>
	using ref = U&&;

	template<typename U>
	using cvref = U&&;
};

struct _copy_cr
{
	template<typename U>
	using ref = U&&;

	template<typename U>
	using cvref = U const&&;
};

template<typename T> _copy _copy_f(T(*)());
template<typename T> _copy_c _copy_f(T const(*)());
template<typename T> _copy_l _copy_f(T&(*)());
template<typename T> _copy_cl _copy_f(T const&(*)());
template<typename T> _copy_r _copy_f(T&&(*)());
template<typename T> _copy_cr _copy_f(T const&&(*)());

template<typename T>
using copy = decltype(_type_traits::_copy_f(static_cast<T(*)()>(0)));
#endif


/* type properties */

template<template<typename...> typename Template, typename T>
struct is_instance_of : std::false_type {};

template<template<typename...> typename Template, typename... Args>
struct is_instance_of<Template, Template<Args...>> : std::true_type {};


template<size_t Size>
struct integer_of_size;

template<>
struct integer_of_size<1>
{
	using signed_type = int8_t;
	using unsigned_type = uint8_t;
};

template<>
struct integer_of_size<2>
{
	using signed_type = int16_t;
	using unsigned_type = uint16_t;
};

template<>
struct integer_of_size<4>
{
	using signed_type = int32_t;
	using unsigned_type = uint32_t;
};

template<>
struct integer_of_size<8>
{
	using signed_type = int64_t;
	using unsigned_type = uint64_t;
};

} // namespace detail::_type_traits

namespace detail {

template<typename T>
struct _delete_arg
{
	template<std::same_as<T> U>
	operator U() const;
};

template<typename T, typename... Args>
concept _has_destroying_delete = requires
{
	T::operator delete(
		_delete_arg<std::remove_cv_t<T>*>(),
		_delete_arg<std::destroying_delete_t>(),
		_delete_arg<Args>()...);
};

} // namespace detail

struct monostate {};


/* type comparison */

template<typename... Ts>
inline constexpr bool all_same_v =
	detail::_type_traits::all_same<(sizeof...(Ts) > 1)>::template value<Ts...>;

template<typename... Ts>
using all_same = std::bool_constant<all_same_v<Ts...>>;

template<typename T, typename... Ts>
inline constexpr bool is_any_of_v = (std::is_same_v<T, Ts> || ...);

template<typename T, typename... Ts>
using is_any_of = std::bool_constant<is_any_of_v<T, Ts...>>;

template<typename T, typename... Ts>
inline constexpr bool is_none_of_v = (!std::is_same_v<T, Ts> && ...);

template<typename T, typename... Ts>
using is_none_of = std::bool_constant<is_none_of_v<T, Ts...>>;


/* conditional */

template<bool Condition, typename True, typename False>
using select_t = typename detail::_type_traits::select<Condition>::template type_template<True, False>;


/* value category removal */

using std::remove_cv_t;
using std::remove_cvref_t;

template<typename T>
using remove_ref_t = std::remove_reference_t<T>;

template<typename T>
using remove_ptr_t = std::remove_pointer_t<T>;

template<typename T>
using remove_cvptr_t = remove_cv_t<remove_ptr_t<T>>;


/* value category copy */

template<typename T, typename U>
using copy_cv_t = typename detail::_type_traits::copy<T>::template cv<U>;

template<typename T, typename U>
using copy_ref_t = typename detail::_type_traits::copy<T>::template ref<U>;

template<typename T, typename U>
using copy_cvref_t = typename detail::_type_traits::copy<T>::template cvref<U>;


/* type properties */

template<typename T>
inline constexpr bool has_destroying_delete_v =
	detail::_has_destroying_delete<T> ||
	detail::_has_destroying_delete<T, std::align_val_t> ||
	detail::_has_destroying_delete<T, size_t> ||
	detail::_has_destroying_delete<T, size_t, std::align_val_t>;

template<typename T>
inline constexpr bool is_inheritable_v = std::is_class_v<T> && !std::is_final_v<T>;

template<typename T>
using is_inheritable = std::bool_constant<is_inheritable_v<T>>;

template<typename T, template<typename...> typename Template>
inline constexpr bool is_instance_of_v = detail::_type_traits::is_instance_of<Template, T>::value;

template<typename T, template<typename...> typename Template>
using is_instance_of = detail::_type_traits::is_instance_of<Template, T>;


/* miscellaneous */

template<size_t Size>
using signed_integer_of_size = typename detail::_type_traits::integer_of_size<Size>::signed_type;

template<size_t Size>
using unsigned_integer_of_size = typename detail::_type_traits::integer_of_size<Size>::unsigned_type;


template<typename T, size_t Budget = 1>
using input_t = select_t<
	std::is_trivially_copyable_v<T> && sizeof(T) <= Budget * sizeof(void*),
	T const,
	T const&>;

} // namespace vsm
