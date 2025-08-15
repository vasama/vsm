#pragma once

#include <vsm/detail/any.hpp>

namespace vsm {
namespace detail {

template<size_t Capacity, typename... Functions>
class _any_new_base<0, Capacity, Functions...>
	: detail::_any_tag
	, _any_new_base_1<0, Capacity, Functions...>
{
	using base_type = _any_new_base_1<0, Capacity, Functions...>;

public:
	_any_new_base() = default;

	template<_non_any T>
		requires
			std::constructible_from<std::decay_t<T>, T> &&
			_any_type_owner_constraint<std::decay_t<T>, Functions...> &&
			_any_inplace_constraint<std::decay_t<T>, Capacity>
	vsm_always_inline _any_new_base(T&& object)
		noexcept(std::is_nothrow_constructible_v<std::decay_t<T>, T>)
	{
		base_type::template _construct_inplace<std::decay_t<T>, Functions...>(vsm_forward(object));
	}

	template<non_ref T, typename... Args>
		requires
			std::constructible_from<T, Args...> &&
			_any_type_owner_constraint<T, Functions...> &&
			_any_inplace_constraint<T, Capacity>
	vsm_always_inline _any_new_base(std::in_place_type_t<T>, Args&&... args)
		noexcept(std::is_nothrow_constructible_v<std::decay_t<T>, T>)
	{
		base_type::template _construct_inplace<T, Functions...>(vsm_forward(args)...);
	}

	template<bool D, size_t C, typename... Fs>
		requires (C <= Capacity) && (D || C < Capacity || sizeof...(Fs) > 0)
	constexpr _any_new_base(_any_new_base<D, C, Functions..., Fs...>&& other) noexcept
		: base_type(_any_subset_t(), vsm_move(other))
	{
	}


	template<_non_any T>
		requires
			std::constructible_from<std::decay_t<T>, T> &&
			_any_type_owner_constraint<std::decay_t<T>, Functions...> &&
			_any_inplace_constraint<std::decay_t<T>, Capacity>
	vsm_always_inline _any_new_base& operator=(T&& object) &
		noexcept(std::is_nothrow_constructible_v<std::decay_t<T>, T>)
	{
		base_type::_destroy_if();

		if constexpr (!std::is_nothrow_constructible_v<std::decay_t<T>, T>)
		{
			base_type::m_functions_and_flags = _any_functions_and_flags();
		}

		base_type::template _construct_inplace<std::decay_t<T>, Functions...>(vsm_forward(object));

		return *this;
	}

	template<bool D, size_t C, typename... Fs>
		requires (C <= Capacity) && (D || C < Capacity || sizeof...(Fs) > 0)
	constexpr _any_new_base& operator=(_any_new_base<D, C, Functions..., Fs...>&& other) & noexcept
	{
		base_type::_assign(vsm_move(other));
		return *this;
	}


	using base_type::operator bool;
	using base_type::invoke;

private:
	template<bool OtherDynamic, size_t OtherCapacity, typename... OtherFunctions>
	friend class _any_new_base;
};

} // namespace detail

template<size_t Capacity, typename... Functions>
struct inplace_any : detail::_any_new_base<0, Capacity, Functions...>
{
	using detail::_any_new_base<0, Capacity, Functions...>::_any_new_base;
};

} // namespace vsm
