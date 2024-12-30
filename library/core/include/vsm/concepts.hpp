#pragma once

#include <vsm/type_traits.hpp>

#include <cstddef>

namespace vsm {

/* type comparison */

template<typename T>
concept always_true = true;

template<typename T>
concept always_false = false;

using std::same_as;

template<typename T, typename U>
concept not_same_as = !same_as<T, U>;

template<typename T, typename... Ts>
concept any_of = is_any_of_v<T, Ts...>;

template<typename T, typename... Ts>
concept none_of = is_none_of_v<T, Ts...>;


/* value category */

template<typename T>
concept non_decaying = same_as<T, std::decay_t<T>>;

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

template<typename T>
concept _utf_character = any_of<T, char8_t, char16_t, char32_t>;

} // namespace detail

template<typename T>
concept pointer = std::is_pointer_v<T>;

template<typename T>
concept object_pointer = pointer<T> && std::is_object_v<remove_ptr_t<T>>;

template<typename T>
concept function_pointer = pointer<T> && std::is_function_v<remove_ptr_t<T>>;

template<typename T>
concept byte_type = vsm::any_of<T, std::byte, char, unsigned char>;

template<typename T>
concept character = std::integral<T> && detail::_character<T>;

template<typename T>
concept utf_character = character<T> && detail::_utf_character<T>;

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

using std::convertible_to;

template<typename T, typename U>
concept nothrow_convertible_to = convertible_to<T, U> && std::is_nothrow_convertible_v<T, U>;

template<typename T, typename U>
concept convertible_from = convertible_to<U, T>;

template<typename T, typename U>
concept nothrow_convertible_from = nothrow_convertible_to<U, T>;

using std::constructible_from;

template<typename T, typename... Args>
concept nothrow_constructible_from =
	constructible_from<T, Args...> && std::is_nothrow_constructible_v<T, Args...>;

// TODO: Rename to explicitly_convertible_to
template<typename T, typename U>
concept constructible_to = std::constructible_from<U, T>;

using std::assignable_from;

template<typename T, typename U>
concept nothrow_assignable_from = assignable_from<T, U> && std::is_nothrow_assignable_v<T, U>;

template<typename T, typename U>
concept assignable_to = assignable_from<U, T>;

template<typename T, typename U>
concept nothrow_assignable_to = nothrow_assignable_from<U, T>;

template<typename T, typename U>
concept lvalue_assignable_from = assignable_from<T&, U>;

template<typename T, typename U>
concept lvalue_nothrow_assignable_from = nothrow_assignable_from<T&, U>;

template<typename T, typename U>
concept assignable_to_lvalue = assignable_to<T, U&>;

template<typename T, typename U>
concept nothrow_assignable_to_lvalue = nothrow_assignable_to<T, U&>;

template<typename T, typename... Args>
concept implicitly_constructible_from =
	std::constructible_from<T, Args...> &&
	requires (void(* f)(T), Args&&... args)
	{
		f({ vsm_forward(args)... });
	};


/* miscellaneous */

template<typename T, typename... Args>
concept constructor_args_for = sizeof...(Args) > 0 && (no_cvref_of<Args, T> && ...);

} // namespace vsm
