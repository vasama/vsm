#pragma once

#include <vsm/type_list.hpp>
#include <vsm/utility.hpp>

namespace vsm {
namespace detail {

enum class value_category
{
	value,
	lvalue_reference,
	rvalue_reference,

	const_value,
	const_lvalue_reference,
	const_rvalue_reference,
};

struct any_function_ptr_parameter;
using any_function_ptr = void(*)(any_function_ptr_parameter);

struct any_signature_parameter;

template<typename Signature>
struct any_signature_info;

#if 0
template<typename R, typename... Ps>
struct any_signature_info<R(any_signature_parameter, Ps...)>
{
	static constexpr int category = value_category::value;

	using context_type = void const;
	using signature_type = R(context_type*, Ps...);

	template<typename T, typename O>
	static R invoke(context_type* const object, Ps... args)
	{
		return O::invoke(*static_cast<T const*>(object), vsm_move(args)...);
	}
};
#endif

template<typename R, typename... Ps>
struct any_signature_info<R(any_signature_parameter&, Ps...)>
{
	using category_type = int&;
	using context_type = void;
	using signature_type = R(context_type*, Ps...);

	template<typename T, typename O>
	static R invoke(context_type* const object, Ps... args)
	{
		return O::invoke(*static_cast<T*>(object), vsm_move(args)...);
	}
};

template<typename R, typename... Ps>
struct any_signature_info<R(any_signature_parameter&&, Ps...)>
{
	using category_type = int&&;
	using context_type = void;
	using signature_type = R(context_type*, Ps...);

	template<typename T, typename O>
	static R invoke(context_type* const object, Ps... args)
	{
		return O::invoke(static_cast<T&&>(*static_cast<T*>(object)), vsm_move(args)...);
	}
};

template<typename R, typename... Ps>
struct any_signature_info<R(any_signature_parameter const&, Ps...)>
{
	using category_type = int const&;
	using context_type = void const;
	using signature_type = R(context_type*, Ps...);

	template<typename T, typename O>
	static R invoke(context_type* const object, Ps... args)
	{
		return O::invoke(*static_cast<T const*>(object), vsm_move(args)...);
	}
};

template<typename R, typename... Ps>
struct any_signature_info<R(any_signature_parameter const&&, Ps...)>
{
	using category_type = int const&&;
	using context_type = void const;
	using signature_type = R(context_type*, Ps...);

	template<typename T, typename O>
	static R invoke(context_type* const object, Ps... args)
	{
		return O::invoke(static_cast<T const&&>(*static_cast<T const*>(object)), vsm_move(args)...);
	}
};

template<typename O>
using any_signature = any_signature_info<decltype(O::invoke<any_signature_parameter>)>;

template<typename T, typename... Os>
inline any_function_ptr const any_vtable[] =
{
	reinterpret_cast<any_function_ptr>(any_signature<Os>::invoke<T, Os>)...
};

template<typename T, typename... Os>
consteval any_function_ptr const* any_vtable_for(type_list<Os...>)
{
	return any_vtable<T, Os...>;
}

} // namespace detail

template<typename... Os>
using any_interface = detail::make_any_interface<Os...>;

} // namespace vsm
