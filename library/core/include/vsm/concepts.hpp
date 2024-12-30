#pragma once

#include <vsm/type_traits.hpp>

namespace vsm {

/* type comparison */

template<typename T>
concept always_true = true;

template<typename T>
concept always_false = false;

using std::same_as;

template<typename T, typename U>
concept not_same_as = !same_as<T, U>;

template<typename T>
concept voidy = same_as<T, void>;

template<typename T, typename... Ts>
concept any_of = is_any_of_v<T, Ts...>;

template<typename T, typename... Ts>
concept none_of = is_none_of_v<T, Ts...>;


/* value category */

template<typename T>
concept non_cv = same_as<T, remove_cv_t<T>>;

template<typename T>
concept non_ref = same_as<T, remove_ref_t<T>>;

template<typename T>
concept non_cvref = same_as<T, remove_cvref_t<T>>;

template<typename T, typename U>
concept any_cv_of = same_as<remove_cv_t<T>, U>;

template<typename T, typename U>
concept any_ref_of = same_as<remove_ref_t<T>, U>;

template<typename T, typename U>
concept any_cvref_of = same_as<remove_cvref_t<T>, U>;

template<typename T, typename U>
concept no_cv_of = not_same_as<remove_cv_t<T>, U>;

template<typename T, typename U>
concept no_ref_of = not_same_as<remove_ref_t<T>, U>;

template<typename T, typename U>
concept no_cvref_of = not_same_as<remove_cvref_t<T>, U>;

template<typename T, typename U>
concept cv_convertible_to =
	not_same_as<T, U> &&
	same_as<remove_cv_t<T>, remove_cv_t<U>> &&
	std::convertible_to<T*, U*>;


/* type properties */

namespace detail {

template<typename T>
concept _character = any_of<T, char, wchar_t, char8_t, char16_t, char32_t>;

} // namespace detail

template<typename T>
concept pointer = std::is_pointer_v<T>;

template<typename T>
concept object_pointer = pointer<T> && std::is_object_v<remove_ptr_t<T>>;

template<typename T>
concept function_pointer = pointer<T> && std::is_function_v<remove_ptr_t<T>>;

template<typename T>
concept character = std::integral<T> && detail::_character<T>;

template<typename T>
concept integer = std::integral<T> && !same_as<T, bool> && !detail::_character<T>;

template<typename T>
concept signed_integer = integer<T> && std::signed_integral<T>;

template<typename T>
concept unsigned_integer = integer<T> && std::unsigned_integral<T>;

template<typename T>
concept enumeration = std::is_enum_v<T>;

template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template<typename T, typename Derived>
concept base_of = std::derived_from<Derived, T>;

template<typename T>
concept inheritable = is_inheritable_v<T>;

template<typename T, typename U>
concept inherited_from = not_same_as<T, U> && std::derived_from<T, U>;

template<typename T, template<typename...> typename Template>
concept instance_of = is_instance_of_v<T, Template>;

template<typename T, template<typename...> typename Template>
concept no_instance_of = !is_instance_of_v<T, Template>;

template<typename From, typename To>
concept losslessly_convertible_to =
	std::convertible_to<From, To> &&
	requires { To{ std::declval<From>() }; };

template<typename T, typename U>
concept convertible_from = std::convertible_to<U, T>;

template<typename T, typename U>
concept constructible_to = std::constructible_from<U, T>;

template<typename T, typename U>
concept assignable_to = std::assignable_from<U, T>;

} // namespace vsm
