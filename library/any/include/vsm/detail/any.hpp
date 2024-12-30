#pragma once

#include <vsm/detail/any_ref.hpp>
#include <vsm/pointer_tag_pair.hpp>
#include <vsm/relocate.hpp>
#include <vsm/standard.hpp>

namespace vsm::detail {

using _any_functions_and_flags = pointer_tag_pair<function_ptr_t const, uintptr_t, 2>;

struct _any_dynamic_destroy_base
{
	void(* m_destroy)(_any_dynamic_destroy_base* base) noexcept;

	static void destroy(void* const context) noexcept
	{
		auto const base = reinterpret_cast<_any_dynamic_destroy_base*>(
			static_cast<unsigned char*>(context) - sizeof(_any_dynamic_destroy_base));

		base->m_destroy(base);
	}
};

template<typename Allocator>
struct _any_dynamic_allocator
{
	vsm_no_unique_address Allocator m_allocator;
};

template<size_t Padding>
struct _any_dynamic_padding
{
	unsigned char m_padding[Padding];
};

template<>
struct _any_dynamic_padding<0>
{
};

template<size_t Alignment, typename T1, typename T2>
struct _any_dynamic_aligned
	: T1
	, _any_dynamic_padding<Alignment - (sizeof(T1) + sizeof(T2)) % Alignment>
	, T2
{
};

template<size_t AllocationSize>
using _any_dynamic_size = std::integral_constant<size_t, AllocationSize>;

template<typename A, size_t Alignment>
struct _any_dynamic_destroy
	: _any_dynamic_aligned<
		Alignment,
		_any_dynamic_allocator<A>,
		_any_dynamic_destroy_base>
{
	using base = _any_dynamic_aligned<
		Alignment,
		_any_dynamic_allocator<A>,
		_any_dynamic_destroy_base>;

	template<typename B, size_t Size>
	vsm_always_inline explicit _any_dynamic_destroy(B&& allocator, _any_dynamic_size<Size>)
		: base{ { vsm_forward(allocator ) }, {}, { destroy_impl<Size> } }
	{
	}

	template<size_t Size>
	static void destroy_impl(_any_dynamic_destroy_base* const base) noexcept
	{
		auto const self = static_cast<_any_dynamic_destroy*>(base);
		A const allocator = vsm_move(self->m_allocator);
		allocator.deallocate(allocation(self, Size));
	}
};

template<typename T>
struct _any_dynamic_context
{
	T m_context;

	template<typename... Args>
	vsm_always_inline explicit _any_dynamic_context(Args&&... args)
		: m_context(vsm_forward(args)...)
	{
		static_assert(offsetof(_any_dynamic_context, m_context) == 0);
	}
};

template<typename A, typename T>
struct _any_dynamic
	: _any_dynamic_destroy<A, alignof(T)>
	, _any_dynamic_context<T>
{
	using destroy_base = _any_dynamic_destroy<A, alignof(T)>;
	using context_base = _any_dynamic_context<T>;

	template<typename B, typename... Args>
	explicit _any_dynamic(B&& allocator, Args&&... args)
		: destroy_base(vsm_forward(allocator), _any_dynamic_size<sizeof(_any_dynamic)>())
		, context_base(vsm_forward(args)...)
	{
		static_assert(offsetof(_any_dynamic, m_allocator) == 0);

		static_assert(
			offsetof(_any_dynamic, m_destroy) + sizeof(_any_dynamic_destroy_base) ==
			offsetof(_any_dynamic, m_context));
	}
};

inline constexpr uintptr_t _any_flag_destroy = 1 << 0;
inline constexpr uintptr_t _any_flag_dynamic = 1 << 1;

template<size_t Capacity>
union _any_union
{
	void* storage_ptr;
	alignas(std::max_align_t) mutable unsigned char storage[Capacity];

	vsm_always_inline void* get_storage(_any_functions_and_flags const& functions_and_flags) const
	{
		return (functions_and_flags.tag() & _any_flag_dynamic) != 0
			? static_cast<void*>(storage_ptr)
			: static_cast<void*>(storage);
	}
};

template<>
union _any_union<0>
{
	void* storage_ptr;

	[[nodiscard]] vsm_always_inline void* get_storage(_any_functions_and_flags const&) const
	{
		return storage_ptr;
	}
};

template<typename T, size_t Capacity>
concept _any_inplace_constraint =
	sizeof(T) <= Capacity &&
	alignof(T) <= alignof(std::max_align_t) &&
	is_trivially_relocatable_v<T>;

struct _any_subset_t
{
	explicit _any_subset_t() = default;
};

template<typename T>
inline constexpr uintptr_t _any_destroy_flag_for = std::is_trivially_destructible_v<T>
	? 0
	: _any_flag_destroy;

template<bool Dynamic, size_t Capacity>
struct _any_base_2
{
	_any_functions_and_flags m_functions_and_flags;
	_any_union<Capacity> m_union;


	_any_base_2() = default;

	template<bool OtherDynamic>
	explicit _any_base_2(
		_any_subset_t,
		_any_base_2<OtherDynamic, Capacity>&& other) noexcept
		: m_functions_and_flags(other.m_functions_and_flags)
		, m_union(other.m_union)
	{
		other.m_functions_and_flags = _any_functions_and_flags();
	}

	template<bool OtherDynamic, size_t OtherCapacity>
	explicit _any_base_2(
		_any_subset_t,
		_any_base_2<OtherDynamic, OtherCapacity>&& other) noexcept
		: m_functions_and_flags(other.m_functions_and_flags)
	{
		std::memcpy(&m_union, &other.m_union, sizeof(other.m_union));
		other.m_functions_and_flags = _any_functions_and_flags();
	}

	_any_base_2(_any_base_2&& other) noexcept
		: m_functions_and_flags(other.m_functions_and_flags)
		, m_union(other.m_union)
	{
		other.m_functions_and_flags = _any_functions_and_flags();
	}

	_any_base_2& operator=(_any_base_2&& other) & noexcept
	{
		if (this != &other)
		{
			_assign(vsm_move(other));
		}
		return *this;
	}

	~_any_base_2()
	{
		_destroy_if();
	}

	friend void swap(_any_base_2& lhs, _any_base_2& rhs) noexcept
	{
		using std::swap;
		swap(lhs.m_functions_and_flags, rhs.m_functions_and_flags);
		swap(lhs.m_union, rhs.m_union);
	}


	[[nodiscard]] explicit operator bool() const noexcept
	{
		return m_functions_and_flags != _any_functions_and_flags();
	}


	template<typename T, typename... Functions, typename... Args>
	void _construct_inplace(Args&&... args)
	{
		static_assert(_any_inplace_constraint<T, Capacity>);

		::new (m_union.storage) T(vsm_forward(args)...);

		m_functions_and_flags = _any_functions_and_flags(
			_any_functions<T, _any_identity, Functions...>,
			_any_destroy_flag_for<T>);
	}

	template<bool OtherDynamic, size_t OtherCapacity>
	void _assign(_any_base_2<OtherDynamic, OtherCapacity>&& other) & noexcept
	{
		static_assert(Capacity >= OtherCapacity);

		_destroy_if();

		m_functions_and_flags = other.m_functions_and_flags;

		if constexpr (Capacity == OtherCapacity)
		{
			m_union = other.m_union;
		}
		else
		{
			std::memcpy(&m_union, &other.m_union, sizeof(other.m_union));
		}

		other.m_functions_and_flags = _any_functions_and_flags();
	}

	void _destroy()
	{
		if ((m_functions_and_flags.tag() & _any_flag_destroy) != 0)
		{
			static_cast<void(*)(void*)>(m_functions_and_flags.pointer()[-1])(
				m_union.get_storage(m_functions_and_flags));
		}

		if constexpr (Dynamic)
		{
			if ((m_functions_and_flags.tag() & _any_flag_dynamic) != 0)
			{
				_any_dynamic_destroy_base::destroy(m_union.storage_ptr);
			}
		}
	}

	vsm_always_inline void _destroy_if()
	{
		if (m_functions_and_flags.tag() != 0)
		{
			_destroy();
		}
	}

	template<bool OtherDynamic, size_t OtherCapacity>
	friend struct _any_base_2;
};

template<bool Dynamic, size_t Capacity, typename... Functions>
struct _any_base_1 : _any_base_2<Dynamic, Capacity>
{
	using base_type = _any_base_2<Dynamic, Capacity>;

	using _any_base_2<Dynamic, Capacity>::_any_base_2;

	template<typename F, typename... Args>
		requires _any_call_constraint<F, int&, Args...>
	[[nodiscard]] typename _any_traits_for<F>::return_type invoke(Args&&... args) &
		noexcept(_any_call_is_nothrow_v<F, int&, Args...>)
	{
		return static_cast<typename _any_traits_for<F>::function_type*>(
			base_type::m_functions_and_flags.pointer()[index_in_pack_v<F, Functions...>])(
			base_type::m_union.get_storage(base_type::m_functions_and_flags),
			vsm_forward(args)...);
	}

	template<typename F, typename... Args>
		requires _any_call_constraint<F, int&&, Args...>
	[[nodiscard]] typename _any_traits_for<F>::return_type invoke(Args&&... args) &&
		noexcept(_any_call_is_nothrow_v<F, int&&, Args...>)
	{
		return static_cast<typename _any_traits_for<F>::function_type*>(
			base_type::m_functions_and_flags.pointer()[index_in_pack_v<F, Functions...>])(
			base_type::m_union.get_storage(base_type::m_functions_and_flags),
			vsm_forward(args)...);
	}

	template<typename F, typename... Args>
		requires _any_call_constraint<F, int const&, Args...>
	[[nodiscard]] typename _any_traits_for<F>::return_type invoke(Args&&... args) const&
		noexcept(_any_call_is_nothrow_v<F, int const&, Args...>)
	{
		return static_cast<typename _any_traits_for<F>::function_type*>(
			base_type::m_functions_and_flags.pointer()[index_in_pack_v<F, Functions...>])(
			base_type::m_union.get_storage(base_type::m_functions_and_flags),
			vsm_forward(args)...);
	}

	template<typename F, typename... Args>
		requires _any_call_constraint<F, int const&&, Args...>
	[[nodiscard]] typename _any_traits_for<F>::return_type invoke(Args&&... args) const&&
		noexcept(_any_call_is_nothrow_v<F, int const&&, Args...>)
	{
		return static_cast<typename _any_traits_for<F>::function_type*>(
			base_type::m_functions_and_flags.pointer()[index_in_pack_v<F, Functions...>])(
			base_type::m_union.get_storage(base_type::m_functions_and_flags),
			vsm_forward(args)...);
	}
};


template<
	size_t Capacity,
	bool Dynamic,
	typename... Functions,
	size_t OtherCapacity,
	typename... OtherFunctions>
void _inplace_any_subset_1(
	_any_base_1<OtherCapacity, Dynamic, Functions..., OtherFunctions...>&&)
	requires (Capacity > OtherCapacity) || (sizeof...(OtherFunctions) != 0);

template<typename Any, bool Dynamic, size_t Capacity, typename... Functions>
concept _inplace_any_subset = requires
{
	detail::_inplace_any_subset_1<Capacity, Functions...>(std::declval<Any>());
};

} // namespace vsm::detail
