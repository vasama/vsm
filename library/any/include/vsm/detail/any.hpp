#pragma once

#include <vsm/detail/any_ref.hpp>
#include <vsm/pointer_tag_pair.hpp>
#include <vsm/relocate.hpp>
#include <vsm/standard.hpp>

namespace vsm::detail {

template<typename T, typename... Functions>
concept _any_type_owner_constraint =
	std::is_destructible_v<T> &&
	_any_type_constraint<T, Functions...>;

using _any_functions_and_flags = pointer_tag_pair<function_ptr_t const, uintptr_t, 2>;

struct _any_new_dynamic_destroy_base
{
	void(* m_destroy)(_any_new_dynamic_destroy_base* base) noexcept;

	static void destroy(void* context) noexcept
	{
		auto const base = reinterpret_cast<_any_new_dynamic_destroy_base*>(
			static_cast<unsigned char*>(context) - sizeof(_any_new_dynamic_destroy_base));

		base->m_destroy(base);
	}
};

template<typename T>
struct _any_new_dynamic_context
{
	T m_context;

	template<typename... Args>
	vsm_always_inline explicit _any_new_dynamic_context(Args&&... args)
		: m_context(vsm_forward(args)...)
	{
		static_assert(offsetof(_any_new_dynamic_context, m_context) == 0);
	}
};

template<typename Allocator>
struct _any_new_dynamic_allocator
{
	vsm_no_unique_address Allocator m_allocator;
};

template<typename Allocator>
struct _any_new_dynamic_destroy
	: _any_new_dynamic_allocator<Allocator>
	, _any_new_dynamic_destroy_base
{
	template<typename A, size_t Size>
	vsm_always_inline explicit _any_new_dynamic_destroy(
		A&& allocator,
		std::integral_constant<size_t, Size>)
		: _any_new_dynamic_allocator<Allocator>(vsm_forward(allocator))
		, _any_new_dynamic_destroy_base(destroy<Size>)
	{
	}

	template<size_t Size>
	static void destroy(_any_new_dynamic_destroy_base* const base) noexcept
	{
		auto const self = static_cast<_any_new_dynamic_destroy*>(base);
		Allocator const allocator = vsm_move(self->m_allocator);
		allocator.deallocate(allocation(self, Size));
	}
};

template<size_t Padding>
struct _any_new_dynamic_padding
{
	unsigned char m_padding[Padding];
};

template<>
struct _any_new_dynamic_padding<0>
{
};

template<typename Allocator, size_t Alignment>
struct _any_new_dynamic_aligned
	: _any_new_dynamic_padding<Alignment - sizeof(_any_new_dynamic_destroy<Allocator>)>
	, _any_new_dynamic_destroy<Allocator>
{
	using _any_new_dynamic_destroy<Allocator>::_any_new_dynamic_destroy;
};

template<typename Allocator, typename T>
struct _any_new_dynamic
	: _any_new_dynamic_aligned<Allocator, alignof(T)>
	, _any_new_dynamic_context<T>
{
	using destroy_base_type = _any_new_dynamic_aligned<Allocator, alignof(T)>;

	template<typename A, typename... Args>
	explicit _any_new_dynamic(A&& allocator, Args&&... args)
		: destroy_base_type(
			vsm_forward(allocator),
			std::integral_constant<size_t, sizeof(_any_new_dynamic)>())
		, _any_new_dynamic_context<T>(vsm_forward(args)...)
	{
		static_assert(
			offsetof(_any_new_dynamic, m_destroy) + sizeof(_any_new_dynamic_destroy_base) ==
			offsetof(_any_new_dynamic, m_context));
	}
};

inline constexpr uintptr_t _any_flag_destroy = 1;
inline constexpr uintptr_t _any_flag_dynamic = 2;

template<size_t Capacity>
union _any_new_union
{
	std::monostate dummy = {};

	void* dynamic;
	mutable alignas(std::max_align_t) unsigned char storage[Capacity];

	vsm_always_inline void* get_storage(_any_functions_and_flags const& functions_and_flags) const
	{
		return functions_and_flags.tag() & _any_flag_dynamic
			? static_cast<void*>(dynamic)
			: static_cast<void*>(storage);
	}
};

template<>
union _any_new_union<0>
{
	std::monostate dummy = {};

	void* dynamic;

	vsm_always_inline void* get_storage(_any_functions_and_flags const&) const
	{
		return dynamic;
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
struct _any_new_base_2
{
	_any_functions_and_flags m_functions_and_flags;
	_any_new_union<Capacity> m_union;

public:
	_any_new_base_2() = default;

	_any_new_base_2(_any_new_base_2&& other) noexcept
		: m_functions_and_flags(other.m_functions_and_flags)
		, m_union(other.m_union)
	{
		other.m_functions_and_flags = _any_functions_and_flags();
	}

	template<bool OtherDynamic>
	explicit _any_new_base_2(
		_any_subset_t,
		_any_new_base_2<OtherDynamic, Capacity>&& other) noexcept
		: m_functions_and_flags(other.m_functions_and_flags)
		, m_union(other.m_union)
	{
		other.m_functions_and_flags = _any_functions_and_flags();
	}

	template<bool OtherDynamic, size_t OtherCapacity>
	explicit _any_new_base_2(
		_any_subset_t,
		_any_new_base_2<OtherDynamic, OtherCapacity>&& other) noexcept
		: m_functions_and_flags(other.m_functions_and_flags)
	{
		std::memcpy(&m_union, &other.m_union, sizeof(other.m_union));
		other.m_functions_and_flags = _any_functions_and_flags();
	}

	_any_new_base_2& operator=(_any_new_base_2&& other) & noexcept
	{
		if (this != &other)
		{
			_assign(vsm_move(other));
		}
		return *this;
	}

	~_any_new_base_2()
	{
		_destroy_if();
	}

	friend void swap(_any_new_base_2& lhs, _any_new_base_2& rhs) noexcept
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
	void _assign(_any_new_base_2<OtherDynamic, OtherCapacity>&& other) & noexcept
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
		if (m_functions_and_flags.tag() & _any_flag_destroy)
		{
			static_cast<void(*)(void*)>(m_functions_and_flags.pointer()[-1])(
				m_union.get_storage(m_functions_and_flags));
		}

		if constexpr (Dynamic)
		{
			if (m_functions_and_flags.tag() & _any_flag_dynamic)
			{
				_any_new_dynamic_destroy_base::destroy(m_union.dynamic);
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
	friend struct _any_new_base_2;
};

template<bool Dynamic, size_t Capacity, typename... Functions>
struct _any_new_base_1 : _any_new_base_2<Dynamic, Capacity>
{
	using base_type = _any_new_base_2<Dynamic, Capacity>;

	using _any_new_base_2<Dynamic, Capacity>::_any_new_base_2;

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
	_any_new_base_1<OtherCapacity, Dynamic, Functions..., OtherFunctions...>&&)
	requires (Capacity > OtherCapacity) || (sizeof...(OtherFunctions) != 0);

template<typename Any, bool Dynamic, size_t Capacity, typename... Functions>
concept _inplace_any_subset = requires
{
	detail::_inplace_any_subset_1<Capacity, Functions...>(std::declval<Any>());
};

} // namespace vsm::detail
