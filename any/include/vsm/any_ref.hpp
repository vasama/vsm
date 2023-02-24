#pragma once

#include <vsm/concepts.hpp>
#include <vsm/type_list.hpp>
#include <vsm/utility.hpp>

#include <bit>

namespace vsm {
namespace detail {

template<
	typename Container,
	typename Value,
	bool = (sizeof(Container) > sizeof(Value))>
	requires (sizeof(Container) >= sizeof(Value))
struct bit_pack_t
{
	Value value;
};

template<typename Container, typename Value>
struct bit_pack_t<Container, Value, true>
{
	Value value;
	unsigned char padding[sizeof(Container) - sizeof(Value)];
};

template<typename Container, typename Value>
	requires
		(sizeof(Container) >= sizeof(Value)) &&
		std::is_trivially_copyable_v<Container> &&
		std::is_trivially_copyable_v<Value>
constexpr Container bit_pack(Value const& value)
{
	return std::bit_cast<Container>(bit_pack_t<Container, Value>(value));
}

template<typename Value, typename Container>
	requires
		(sizeof(Container) >= sizeof(Value)) &&
		std::is_trivially_copyable_v<Container> &&
		std::is_trivially_copyable_v<Value>
constexpr Value bit_unpack(Container const& container)
{
	return std::bit_cast<bit_pack_t<Container, Value>>(container).value;
}


template<typename Signature>
struct _any_traits;

template<typename R, typename... Ps>
struct _any_traits<R(Ps...)>
{
	template<typename T>
	using member_type = R(T::*)(Ps...);

	void _i(Ps...);

	template<typename F, typename T>
	static constexpr bool requirement = requires
	{
		{ F::invoke(std::declval<T>(), std::declval<Ps>()...) } -> std::convertible_to<R>;
	};

	using context_type = void;
	using function_type = R(context_type*, Ps...);

	template<typename F, typename T, bool Packed>
	static R invoke(context_type* const context, Ps... args)
	{
		if constexpr (Packed)
		{
			auto const object = bit_unpack<T>(context);
			return F::invoke(object, vsm_move(args)...);
		}
		else
		{
			return F::invoke(*static_cast<T*>(context), vsm_move(args)...);
		}
	}
};

template<typename R, typename... Ps>
struct _any_traits<R(Ps...) const>
{
	template<typename T>
	using member_type = R(T::*)(Ps...) const;

	void _i(Ps...) const;

	template<typename F, typename T>
	static constexpr bool requirement = requires
	{
		{ F::invoke(std::declval<T>(), std::declval<Ps>()...) } -> std::convertible_to<R>;
	};

	using context_type = void;
	using function_type = R(context_type*, Ps...);

	template<typename F, typename T, bool Packed>
	static R invoke(context_type* const context, Ps... args)
	{
		if constexpr (Packed)
		{
			auto const object = bit_unpack<T>(context);
			return F::invoke(object, vsm_move(args)...);
		}
		else
		{
			return F::invoke(*static_cast<T const*>(context), vsm_move(args)...);
		}
	}
};

//TODO: Value categories

template<typename F>
using _any_traits_for = _any_traits<typename F::signature_type>;

using _any_function = void(struct _any_parameter);

template<typename... Fs>
struct _any_functions
{
	template<typename T>
	static constexpr bool requirement = (_any_traits_for<Fs>::template requirement<Fs, T> && ...);

	static constexpr bool fully_const =
		(std::is_const_v<typename _any_traits_for<Fs>::context_type> && ...);

	template<typename T, bool Packed = false>
	static _any_function* const table[sizeof...(Fs)];
};

template<typename T, bool Packed, typename... Fs>
inline _any_function* const _any_function_table[sizeof...(Fs)] =
{
	reinterpret_cast<_any_function*>(_any_traits_for<Fs>::template invoke<Fs, T, Packed>)...
};

template<typename... Fs>
template<typename T, bool Packed>
_any_function* const _any_functions<Fs...>::table[sizeof...(Fs)] =
{
	reinterpret_cast<_any_function*>(_any_traits_for<Fs>::template invoke<Fs, T, Packed>)...
};

template<typename F, typename Cvref, typename... Args>
concept _any_invocable =
	// requires (typename _any_traits_for<F>::template member_type<remove_cvref_t<Cvref>> member)
	requires
	{
		// (std::declval<Cvref>().*member)(std::declval<Args>()...);
		std::declval<copy_cvref_t<Cvref, _any_traits_for<F>>>()._i(std::declval<Args>()...);
	};

} // namespace detail

template<typename... Functions>
class any_ref
{
	using functions_type = detail::_any_functions<Functions...>;
	using context_type = select_t<functions_type::fully_const, void const, void>;

	detail::_any_function* const* m_functions;
	context_type* m_context;

public:
	template<typename T>
	constexpr any_ref(T&& object)
		requires
			no_instance_of<remove_cvref_t<T>, any_ref> &&
			functions_type::template requirement<T&&>
		: m_functions(functions_type::template table<remove_cvref_t<T>>)
		, m_context(std::addressof(object))
	{
	}

	template<typename T>
	constexpr any_ref(std::in_place_t, T const& object)
		requires
			no_instance_of<T, any_ref> &&
			functions_type::template requirement<T const&> &&
			(sizeof(m_context) >= sizeof(T)) && std::is_trivially_copyable_v<T>
		: m_functions(functions_type::template table<T, /* Packed: */ true>)
		, m_context(detail::bit_pack<context_type*>(object))
	{
	}

#if 0
	template<non_cvref T>
	constexpr any_ref(std::in_place_type_t<T>)
		requires
			// no_instance_of<T, any_ref> &&
			std::is_empty_v<T> &&
			functions_type::template requirement<T>
		: m_functions(functions_type::template table<T, /* Packed: */ true>)
	{
	}
#endif

	template<typename... ExtraFunctions>
	constexpr any_ref(any_ref<Functions..., ExtraFunctions...> const& other)
		requires (sizeof...(ExtraFunctions) > 0)
		: m_functions(other.m_functions)
		, m_context(other.m_context)
	{
	}

	template<typename F, typename... Args>
	[[nodiscard]] constexpr auto invoke(Args&&... args) const&
		requires detail::_any_invocable<F, any_ref&, Args...>
	{
		auto const function = m_functions[pack_index<F, Functions...>];
		using function_type = typename detail::_any_traits_for<F>::function_type;
		return reinterpret_cast<function_type*>(function)(m_context, vsm_forward(args)...);
	}

	template<typename F, typename... Args>
	[[nodiscard]] constexpr auto invoke(Args&&... args) const&&
		requires detail::_any_invocable<F, any_ref&&, Args...>
	{
		auto const function = m_functions[pack_index<F, Functions...>];
		using function_type = typename detail::_any_traits_for<F>::function_type;
		return reinterpret_cast<function_type*>(function)(m_context, vsm_forward(args)...);
	}

private:
	template<typename...>
	friend class any_ref;
};

} // namespace vsm
