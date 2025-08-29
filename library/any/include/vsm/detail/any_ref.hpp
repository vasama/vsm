#pragma once

#include <vsm/concepts.hpp>
#include <vsm/detail/function_ptr.hpp>
#include <vsm/type_list.hpp>
#include <vsm/utility.hpp>

namespace vsm::detail {

struct _any_tag {};

template<typename T>
concept _non_any = !std::derived_from<remove_cvref_t<T>, _any_tag>;

template<typename T, typename F, typename R, typename... Ps>
concept _any_type_constraint_2 = requires
{
	{ F::invoke(std::declval<T>(), std::declval<Ps>()...) } -> convertible_to<R>;
};

template<typename T, typename F, typename R, typename... Ps>
concept _any_type_nothrow_constraint_2 = requires
{
	{ F::invoke(std::declval<T>(), std::declval<Ps>()...) } noexcept -> nothrow_convertible_to<R>;
};

template<typename Signature>
struct _any_traits;

#if 1 // auto-generated

template<typename R, typename... Ps>
struct _any_traits<R(Ps...)>
{
	template<typename T, typename F>
	static void type_constraint() requires _any_type_constraint_2<T&, F, R, Ps...>;

	void call_constraint(Ps...);

	using return_type = R;
	using function_type = R(void const*, Ps...);

	template<typename Transform, typename T, typename F>
	static R invoke(void const* const context, Ps... args)
	{
		return F::invoke(
			const_cast<T&>(
				*static_cast<T const*>(
					static_cast<void const*>(Transform::apply(context)))),
			vsm_move(args)...);
	}
};


template<typename R, typename... Ps>
struct _any_traits<R(Ps...) &>
{
	template<typename T, typename F>
	static void type_constraint() requires _any_type_constraint_2<T&, F, R, Ps...>;

	void call_constraint(Ps...) &;

	using return_type = R;
	using function_type = R(void const*, Ps...);

	template<typename Transform, typename T, typename F>
	static R invoke(void const* const context, Ps... args)
	{
		return F::invoke(
			const_cast<T&>(
				*static_cast<T const*>(
					static_cast<void const*>(Transform::apply(context)))),
			vsm_move(args)...);
	}
};


template<typename R, typename... Ps>
struct _any_traits<R(Ps...) &&>
{
	template<typename T, typename F>
	static void type_constraint() requires _any_type_constraint_2<T&&, F, R, Ps...>;

	void call_constraint(Ps...) &&;

	using return_type = R;
	using function_type = R(void const*, Ps...);

	template<typename Transform, typename T, typename F>
	static R invoke(void const* const context, Ps... args)
	{
		return F::invoke(
			const_cast<T&&>(
				*static_cast<T const*>(
					static_cast<void const*>(Transform::apply(context)))),
			vsm_move(args)...);
	}
};


template<typename R, typename... Ps>
struct _any_traits<R(Ps...) const>
{
	template<typename T, typename F>
	static void type_constraint() requires _any_type_constraint_2<T const&, F, R, Ps...>;

	void call_constraint(Ps...) const;

	using return_type = R;
	using function_type = R(void const*, Ps...);

	template<typename Transform, typename T, typename F>
	static R invoke(void const* const context, Ps... args)
	{
		return F::invoke(
			const_cast<T const&>(
				*static_cast<T const*>(
					static_cast<void const*>(Transform::apply(context)))),
			vsm_move(args)...);
	}
};


template<typename R, typename... Ps>
struct _any_traits<R(Ps...) const&>
{
	template<typename T, typename F>
	static void type_constraint() requires _any_type_constraint_2<T const&, F, R, Ps...>;

	void call_constraint(Ps...) const&;

	using return_type = R;
	using function_type = R(void const*, Ps...);

	template<typename Transform, typename T, typename F>
	static R invoke(void const* const context, Ps... args)
	{
		return F::invoke(
			const_cast<T const&>(
				*static_cast<T const*>(
					static_cast<void const*>(Transform::apply(context)))),
			vsm_move(args)...);
	}
};


template<typename R, typename... Ps>
struct _any_traits<R(Ps...) const&&>
{
	template<typename T, typename F>
	static void type_constraint() requires _any_type_constraint_2<T const&&, F, R, Ps...>;

	void call_constraint(Ps...) const&&;

	using return_type = R;
	using function_type = R(void const*, Ps...);

	template<typename Transform, typename T, typename F>
	static R invoke(void const* const context, Ps... args)
	{
		return F::invoke(
			const_cast<T const&&>(
				*static_cast<T const*>(
					static_cast<void const*>(Transform::apply(context)))),
			vsm_move(args)...);
	}
};


template<typename R, typename... Ps>
struct _any_traits<R(Ps...) noexcept>
{
	template<typename T, typename F>
	static void type_constraint() requires _any_type_nothrow_constraint_2<T&, F, R, Ps...>;

	void call_constraint(Ps...) noexcept;

	using return_type = R;
	using function_type = R(void const*, Ps...) noexcept;

	template<typename Transform, typename T, typename F>
	static R invoke(void const* const context, Ps... args)
	{
		return F::invoke(
			const_cast<T&>(
				*static_cast<T const*>(
					static_cast<void const*>(Transform::apply(context)))),
			vsm_move(args)...);
	}
};


template<typename R, typename... Ps>
struct _any_traits<R(Ps...) & noexcept>
{
	template<typename T, typename F>
	static void type_constraint() requires _any_type_nothrow_constraint_2<T&, F, R, Ps...>;

	void call_constraint(Ps...) & noexcept;

	using return_type = R;
	using function_type = R(void const*, Ps...) noexcept;

	template<typename Transform, typename T, typename F>
	static R invoke(void const* const context, Ps... args)
	{
		return F::invoke(
			const_cast<T&>(
				*static_cast<T const*>(
					static_cast<void const*>(Transform::apply(context)))),
			vsm_move(args)...);
	}
};


template<typename R, typename... Ps>
struct _any_traits<R(Ps...) && noexcept>
{
	template<typename T, typename F>
	static void type_constraint() requires _any_type_nothrow_constraint_2<T&&, F, R, Ps...>;

	void call_constraint(Ps...) && noexcept;

	using return_type = R;
	using function_type = R(void const*, Ps...) noexcept;

	template<typename Transform, typename T, typename F>
	static R invoke(void const* const context, Ps... args)
	{
		return F::invoke(
			const_cast<T&&>(
				*static_cast<T const*>(
					static_cast<void const*>(Transform::apply(context)))),
			vsm_move(args)...);
	}
};


template<typename R, typename... Ps>
struct _any_traits<R(Ps...) const noexcept>
{
	template<typename T, typename F>
	static void type_constraint() requires _any_type_nothrow_constraint_2<T const&, F, R, Ps...>;

	void call_constraint(Ps...) const noexcept;

	using return_type = R;
	using function_type = R(void const*, Ps...) noexcept;

	template<typename Transform, typename T, typename F>
	static R invoke(void const* const context, Ps... args)
	{
		return F::invoke(
			const_cast<T const&>(
				*static_cast<T const*>(
					static_cast<void const*>(Transform::apply(context)))),
			vsm_move(args)...);
	}
};


template<typename R, typename... Ps>
struct _any_traits<R(Ps...) const& noexcept>
{
	template<typename T, typename F>
	static void type_constraint() requires _any_type_nothrow_constraint_2<T const&, F, R, Ps...>;

	void call_constraint(Ps...) const& noexcept;

	using return_type = R;
	using function_type = R(void const*, Ps...) noexcept;

	template<typename Transform, typename T, typename F>
	static R invoke(void const* const context, Ps... args)
	{
		return F::invoke(
			const_cast<T const&>(
				*static_cast<T const*>(
					static_cast<void const*>(Transform::apply(context)))),
			vsm_move(args)...);
	}
};


template<typename R, typename... Ps>
struct _any_traits<R(Ps...) const&& noexcept>
{
	template<typename T, typename F>
	static void type_constraint() requires _any_type_nothrow_constraint_2<T const&&, F, R, Ps...>;

	void call_constraint(Ps...) const&& noexcept;

	using return_type = R;
	using function_type = R(void const*, Ps...) noexcept;

	template<typename Transform, typename T, typename F>
	static R invoke(void const* const context, Ps... args)
	{
		return F::invoke(
			const_cast<T const&&>(
				*static_cast<T const*>(
					static_cast<void const*>(Transform::apply(context)))),
			vsm_move(args)...);
	}
};

#endif // auto-generated

template<typename T>
using _any_traits_for = _any_traits<typename T::signature_type>;

template<typename T, typename F>
concept _any_type_constraint_1 = requires { _any_traits_for<F>::template type_constraint<T, F>(); };

template<typename T, typename... Fs>
concept _any_type_constraint = (_any_type_constraint_1<T, Fs> && ...);

template<typename F, typename Q, typename... Args>
concept _any_call_constraint = requires
{
	std::declval<copy_cvref_t<Q, _any_traits_for<F>>>().call_constraint(std::declval<Args>()...);
};

template<typename F, typename Q, typename... Args>
concept _any_call_is_nothrow_v = noexcept(
	std::declval<copy_cvref_t<Q, _any_traits_for<F>>>().call_constraint(std::declval<Args>()...));

template<typename T>
void _any_destroy(void* const context)
{
	if constexpr (std::is_destructible_v<T>)
	{
		static_cast<T*>(context)->~T();
	}
}

template<non_ref T, typename Transform, typename... Fs>
inline function_ptr_t const _any_functions_array[] =
{
	_any_destroy<T>,
	_any_traits_for<Fs>::template invoke<Transform, T, Fs>...
};

template<typename T, typename Transform, typename... Fs>
inline constexpr function_ptr_t const* _any_functions =
	_any_functions_array<T, Transform, Fs...> + 1;

struct _any_identity
{
	static void const* apply(void const* const context) noexcept
	{
		return context;
	}
};

template<bool Dynamic, size_t Capacity, typename... Functions>
class _any_new_base;

} // namespace vsm::detail
