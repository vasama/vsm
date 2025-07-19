#pragma once

#include <vsm/concepts.hpp>
#include <vsm/detail/function_ptr.hpp>
#include <vsm/utility.hpp>

namespace vsm {
namespace detail {

struct _any_tag {};

template<typename T>
concept _non_any = !std::derived_from<remove_cvref_t<T>, _any_tag>;

template<typename T, typename F, typename R, typename... Ps>
concept _any_type_constraint_e = requires
{
	{ F::invoke(std::declval<T>(), std::declval<Ps>()...) } -> std::convertible_to<R>;
};

template<typename T, typename F, typename R, typename... Ps>
concept _any_type_constraint_n = requires
{
	{ F::invoke(std::declval<T>(), std::declval<Ps>()...) } noexcept -> std::convertible_to<R>;
};

template<typename Signature>
struct _any_traits;

template<typename R, typename... Ps>
struct _any_traits<R(Ps...)>
{
	static constexpr bool is_noexcept = false;

	template<typename F, typename T>
	void type_constraint() requires _any_type_constraint_e<T, F, R, Ps...>;

	void call_constraint(Ps...);

	using return_type = R;
	using function_type = R(void const*, Ps...);

	template<typename F, typename T, bool Packed>
	static R invoke(void const* const context, Ps... args)
	{
		if constexpr (Packed)
		{
			return F::invoke(static_cast<T const&>(bit_unpack<T>(context)), vsm_move(args)...);
		}
		else
		{
			return F::invoke(const_cast<T&>(*static_cast<T const*>(context)), vsm_move(args)...);
		}
	}
};

template<typename T>
using _any_traits_for = _any_traits<typename T::signature_type>;

template<typename T, typename F>
concept _any_type_constraint_1 = requires { _any_traits_for<F>::template constraint<F, T>(); };

template<typename T, typename... Fs>
concept _any_type_constraint = (_any_type_constraint_1<T, Fs> && ...);

template<typename F, typename Q, typename... Args>
concept _any_call_constraint = requires
{
	std::declval<copy_cvref_t<Q, _any_traits_for<F>>>().call_constraint(std::declval<Args>()...);
};

template<typename T, bool Packed, typename... Fs>
inline constexpr function_ptr_t _any_functions[] =
{
	_any_traits_for<Fs>::template invoke<Fs, T, Packed>...
};

template<size_t Capacity>
struct _any_storage
{
	alignas(std::max_align_t) unsigned char storage[Capacity];

	constexpr _any_storage()
	{
		vsm_if_consteval
		{
			for (size_t i = 0; i < Capacity; ++i)
			{
				storage[i] = 0;
			}
		}
	}
};

template<size_t Capacity, typename... Functions>
struct _any_base
{
	function_ptr_t const* m_functions = nullptr;
	_any_storage<Capacity> m_storage;

	_any_base() = default;

	_any_base(_any_base&& other) noexcept
		: m_functions(other.m_functions)
	{
		if (other.m_functions != nullptr)
		{
			other.m_functions->relocate(other.m_storage.storage, m_storage.storage);
			other.m_functions = nullptr;
		}
	}

	_any_base& operator=(_any_base&& other) & noexcept
	{
		if (m_functions != nullptr)
		{
			m_functions->destroy(m_storage.storage);
			m_functions = nullptr;
		}

		if (other.m_functions != nullptr)
		{
			other.m_functions->relocate(other.m_storage.storage, m_storage.storage);
			m_functions = other.m_functions;
			other.m_functions = nullptr;
		}

		return *this;
	}

	~_any_base()
	{
		if (m_functions != nullptr)
		{
			m_functions->destroy(m_storage.storage);
		}
	}
};

template<typename F, typename... Functions, typename... Args>
vsm_always_inline typename _any_traits_for<F>::return_type _any_invoke(
	void const* const context,
	function_ptr_t const* const functions,
	Args&&... args)
{
	using function_type = typename _any_traits_for<F>::function_type;
	constexpr size_t index = index_in_pack_v<F, Functions...>
	return static_cast<function_type*>(functions[index])(context, vsm_forward(args)...);
}

} // namespace detail

template<size_t Capacity, typename... Functions>
class inplace_any
	: public detail::_any_tag
	, detail::_any_base<Capacity, Functions...>
{
	using base_type = detail::_any_base<Capacity, Functions...>;

public:
	template<detail::_non_any T>
		requires detail::_any_type_constraint<std::decay_t<T>, Functions...>
	constexpr inplace_any(T&& object)
	{
		base_type::template construct<std::decay_t<T>>(vsm_forward(object));
	}

	template<detail::_non_any T, typename... Args>
		requires
			non_decaying<T> &&
			std::constructible_from<T, Args...> &&
			detail::_any_type_constraint<T, Functions...>
	constexpr inplace_any(std::in_place_type_t<T>, Args&&... args)
	{
		base_type::template construct<T>(vsm_forward(args)...);
	}


	template<typename F, typename... Args>
		requires detail::_any_call_constraint<F, int&, Args...>
	[[nodiscard]] typename detail::_any_traits_for<F>::return_type invoke(Args&&... args) &
		noexcept(detail::_any_traits_for<F>::is_noexcept)
	{
		using function_type = typename detail::_any_traits_for<F>::function_type;
		constexpr size_t index = index_in_pack_v<F, Functions...>;
		return static_cast<function_type*>(base_type::m_functions[index])(
			base_type::m_storage.storage,
			vsm_forward(args)...);
	}

	template<typename F, typename... Args>
		requires detail::_any_call_constraint<F, int const&, Args...>
	[[nodiscard]] typename detail::_any_traits_for<F>::return_type invoke(Args&&... args) const&
		noexcept(detail::_any_traits_for<F>::is_noexcept)
	{
		using function_type = typename detail::_any_traits_for<F>::function_type;
		constexpr size_t index = index_in_pack_v<F, Functions...>;
		return static_cast<function_type*>(base_type::m_functions[index])(
			base_type::m_storage.storage,
			vsm_forward(args)...);
	}

	template<typename F, typename... Args>
		requires detail::_any_call_constraint<F, int&&, Args...>
	[[nodiscard]] typename detail::_any_traits_for<F>::return_type invoke(Args&&... args) &&
		noexcept(detail::_any_traits_for<F>::is_noexcept)
	{
		using function_type = typename detail::_any_traits_for<F>::function_type;
		constexpr size_t index = index_in_pack_v<F, Functions...>;
		return static_cast<function_type*>(base_type::m_functions[index])(
			base_type::m_storage.storage,
			vsm_forward(args)...);
	}

	template<typename F, typename... Args>
		requires detail::_any_call_constraint<F, int const&&, Args...>
	[[nodiscard]] typename detail::_any_traits_for<F>::return_type invoke(Args&&... args) const&&
		noexcept(detail::_any_traits_for<F>::is_noexcept)
	{
		using function_type = typename detail::_any_traits_for<F>::function_type;
		constexpr size_t index = index_in_pack_v<F, Functions...>;
		return static_cast<function_type*>(base_type::m_functions[index])(
			base_type::m_storage.storage,
			vsm_forward(args)...);
	}
};

} // namespace vsm
