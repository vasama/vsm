#pragma once

#include <vsm/concepts.hpp>
#include <vsm/detail/function_view.hpp>
#include <vsm/utility.hpp>

#include <memory>

namespace vsm {

namespace detail {

struct nontype_base_t {};

} // namespace detail

template<auto Value>
struct nontype_t : detail::nontype_base_t
{
	explicit nontype_t() = default;
};

template<auto Value>
inline constexpr nontype_t<Value> nontype{};


namespace detail {

template<bool N>
struct is_invocable_using_impl;

template<>
struct is_invocable_using_impl<0>
{
	template<typename T, typename R, typename... Ps>
	static constexpr bool value = std::is_invocable_r_v<R, T, Ps...>;
};

template<>
struct is_invocable_using_impl<1>
{
	template<typename T, typename R, typename... Ps>
	static constexpr bool value = std::is_nothrow_invocable_r_v<R, T, Ps...>;
};

template<typename T, bool N, typename R, typename... Ps>
inline constexpr bool is_invocable_using = is_invocable_using_impl<N>::template value<T, R, Ps...>;

template<bool C, typename T>
using apply_const = select_t<C, T const, T>;


template<bool C, bool N, typename R, typename... Ps>
class function_view_impl
{
	R(*m_invoke)(function_context context, Ps... args) noexcept(N);
	function_context m_context;

public:
	/// @brief Construct from a reference to a callable object.
	template<no_cvref_of<function_view_impl> T>
		requires (
			is_invocable_using<apply_const<C, std::remove_reference_t<T>>&, N, R, Ps...>
			&& !std::is_function_v<std::remove_reference_t<T>>
			&& !std::is_member_pointer_v<std::remove_reference_t<T>>
		)
	constexpr function_view_impl(T&& object)
		: m_invoke(invoke_object<N, apply_const<C, std::remove_reference_t<T>>, R, Ps...>)
		, m_context{ .object = std::addressof(object) }
	{
	}

	/// @brief Construct from a reference to a function.
	template<typename F>
		requires
			is_invocable_using<F, N, R, Ps...>
			&& std::is_function_v<F>
	vsm_detail_function_ptr_constexpr function_view_impl(F& function)
		: m_invoke(invoke_function<N, F, R, Ps...>)
		, m_context{ .function = &function }
	{
	}

	/// @brief Construct from a pointer to a function.
	template<typename F>
		requires
			is_invocable_using<F*, N, R, Ps...>
			&& std::is_function_v<F>
	vsm_detail_function_ptr_constexpr function_view_impl(F* const function)
		: m_invoke(invoke_function<N, F, R, Ps...>)
		, m_context{ .function = function }
	{
	}

	/// @brief Construct from a nontype_t naming a function.
	template<auto F>
		requires is_invocable_using<decltype(F), N, R, Ps...>
	constexpr function_view_impl(nontype_t<F>)
		: m_invoke(invoke_nontype_unused<N, F, R, Ps...>)
		, m_context{ .unused = {} }
	{
	}

	/// @brief Construct from a nontype_t naming a function and a reference to an object.
	template<auto F, typename T>
		requires
			is_invocable_using<decltype(F), N, R, apply_const<C, std::remove_reference_t<T>>&, Ps...>
			&& std::is_convertible_v<T&&, apply_const<C, std::remove_reference_t<T>>&>
	constexpr function_view_impl(nontype_t<F>, T&& object)
		: m_invoke(invoke_nontype_object<N, F, apply_const<C, std::remove_reference_t<T>>, R, Ps...>)
		, m_context{ .object = &object }
	{
	}

	/// @brief Construct from a nontype_t naming a function and a pointer to an object.
	template<auto F, typename T>
		requires
			is_invocable_using<decltype(F), N, R, apply_const<C, T>*, Ps...>
			&& std::is_convertible_v<T*, apply_const<C, T>*>
	constexpr function_view_impl(nontype_t<F>, T* const object)
		: m_invoke(invoke_nontype_object<N, F, apply_const<C, T>*, R, Ps...>)
		, m_context{ .object = object }
	{
	}

	function_view_impl(function_view_impl const&) noexcept = default;
	function_view_impl& operator=(function_view_impl const&) & noexcept = default;

	template<no_cvref_of<function_view_impl> T>
		requires
			(!std::is_pointer_v<T>)
			&& (!std::is_base_of_v<nontype_base_t, T>)
	function_view_impl& operator=(T) = delete;


	//TODO: && signature should only be && invocable
	template<std::convertible_to<Ps>... Args>
	vsm_detail_function_ptr_constexpr R operator()(Args&&... args) const noexcept(N)
	{
		return m_invoke(m_context, vsm_forward(args)...);
	}
};

} // namespace detail

template<typename Signature>
struct function_view;

template<typename R, typename... Ps>
struct function_view<R(Ps...)>
	: detail::function_view_impl<0, 0, R, Ps...>
{
	using detail::function_view_impl<0, 0, R, Ps...>::function_view_impl;
};

template<typename R, typename... Ps>
struct function_view<R(Ps...) const>
	: detail::function_view_impl<1, 0, R, Ps...>
{
	using detail::function_view_impl<1, 0, R, Ps...>::function_view_impl;
};

template<typename R, typename... Ps>
struct function_view<R(Ps...) noexcept>
	: detail::function_view_impl<0, 1, R, Ps...>
{
	using detail::function_view_impl<0, 1, R, Ps...>::function_view_impl;
};

template<typename R, typename... Ps>
struct function_view<R(Ps...) const noexcept>
	: detail::function_view_impl<1, 1, R, Ps...>
{
	using detail::function_view_impl<1, 1, R, Ps...>::function_view_impl;
};

} // namespace vsm
