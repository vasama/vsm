#pragma once

#include <vsm/allocator.hpp>
#include <vsm/detail/any.hpp>

namespace vsm {

template<typename... Functions>
class any_ref;

namespace detail {

template<typename T, size_t Capacity, typename... Args>
concept _any_nothrow_constraint =
	_any_inplace_constraint<T, Capacity> &&
	std::is_nothrow_constructible_v<T, Args...>;

template<size_t Capacity, typename... Functions>
class _any_new_base</* Dynamic: */ true, Capacity, Functions...>
	: public detail::_any_tag
	, _any_base_1</* Dynamic: */ true, Capacity, Functions...>
{
	using base_type = _any_base_1</* Dynamic: */ true, Capacity, Functions...>;

public:
	_any_new_base() = default;

	template<_non_any T>
		requires
			std::constructible_from<std::decay_t<T>, T> &&
			_any_type_constraint<std::decay_t<T>, Functions...>
	vsm_always_inline _any_new_base(T&& object)
		noexcept(_any_nothrow_constraint<std::decay_t<T>, Capacity, T>)
	{
		_construct<std::decay_t<T>>(default_allocator(), vsm_forward(object));
	}

	template<allocator Allocator, _non_any T>
		requires
			std::constructible_from<std::decay_t<T>, T> &&
			_any_type_constraint<std::decay_t<T>, Functions...>
	vsm_always_inline _any_new_base(Allocator&& allocator, T&& object)
		noexcept(_any_nothrow_constraint<std::decay_t<T>, Capacity, T>)
	{
		_construct<std::decay_t<T>>(vsm_forward(allocator), vsm_forward(object));
	}

	template<non_ref T, typename... Args>
		requires
			std::constructible_from<T, Args...> &&
			_any_type_constraint<T, Functions...>
	vsm_always_inline _any_new_base(std::in_place_type_t<T>, Args&&... args)
		noexcept(_any_nothrow_constraint<T, Capacity, Args...>)
	{
		_construct<T>(default_allocator(), vsm_forward(args)...);
	}

	template<allocator Allocator, non_ref T, typename... Args>
		requires
			std::constructible_from<T, Args...> &&
			_any_type_constraint<T, Functions...>
	vsm_always_inline _any_new_base(Allocator&& allocator, std::in_place_type_t<T>, Args&&... args)
		noexcept(_any_nothrow_constraint<T, Capacity, Args...>)
	{
		_construct<T>(vsm_forward(allocator), vsm_forward(args)...);
	}

	template<bool D, size_t C, typename... Fs>
		requires (C <= Capacity) && (!D || C < Capacity || sizeof...(Fs) > 0)
	constexpr _any_new_base(_any_new_base<D, C, Functions..., Fs...>&& other) noexcept
		: base_type(_any_subset_t(), vsm_move(other))
	{
	}


	template<_non_any T>
		requires
			std::constructible_from<std::decay_t<T>, T> &&
			_any_type_constraint<std::decay_t<T>, Functions...>
	vsm_always_inline _any_new_base& operator=(T&& object) &
		noexcept(_any_nothrow_constraint<std::decay_t<T>, Capacity, T>)
	{
		base_type::_destroy_if();

		if constexpr (!_any_nothrow_constraint<std::decay_t<T>, Capacity, T>)
		{
			base_type::m_functions_and_flags = _any_functions_and_flags();
		}

		_construct<std::decay_t<T>>(default_allocator(), vsm_forward(object));

		return *this;
	}

	template<bool D, size_t C, typename... Fs>
		requires (C <= Capacity) && (!D || C < Capacity || sizeof...(Fs) > 0)
	constexpr _any_new_base& operator=(_any_new_base<D, C, Functions..., Fs...>&& other) noexcept
	{
		base_type::_assign(vsm_move(other));
		return *this;
	}


	using base_type::operator bool;
	using base_type::invoke;

private:
	template<typename T, typename Allocator, typename... Args>
	void _construct(Allocator&& allocator, Args&&... args)
	{
		if constexpr (_any_inplace_constraint<T, Capacity>)
		{
			base_type::template _construct_inplace<T, Functions...>(vsm_forward(args)...);
		}
		else
		{
			using dynamic_type = _any_dynamic<std::decay_t<Allocator>, T>;

			auto const storage_ptr = vsm::new_via<dynamic_type>(
				vsm_as_const(allocator),
				vsm_forward(allocator),
				vsm_forward(args)...);

			base_type::m_union.storage_ptr = std::addressof(storage_ptr->m_context);

			base_type::m_functions_and_flags = _any_functions_and_flags(
				_any_functions<T, _any_identity, Functions...>,
				_any_flag_dynamic | _any_destroy_flag_for<T>);
		}
	}

	template<bool OtherDynamic, size_t OtherCapacity, typename... OtherFunctions>
	friend class _any_new_base;

	template<typename... OtherFunctions>
	friend class vsm::any_ref;
};

} // namespace detail

template<size_t Capacity, typename... Functions>
struct basic_any : detail::_any_new_base</* Dynamic: */ true, Capacity, Functions...>
{
	using detail::_any_new_base</* Dynamic: */ true, Capacity, Functions...>::_any_new_base;
};

template<typename... Functions>
using any = basic_any<16, Functions...>;

} // namespace vsm
