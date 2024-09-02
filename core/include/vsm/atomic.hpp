#pragma once

#include <vsm/concepts.hpp>
#include <vsm/detail/categories.hpp>
#include <vsm/platform.h>
#include <vsm/preprocessor.h>
#include <vsm/utility.hpp>

#if __has_include(vsm_pp_include(vsm/detail/atomic/vsm_arch.hpp))
#	include vsm_pp_include(vsm/detail/atomic/vsm_arch.hpp)
#else
#	define vsm_detail_atomic_ref_base(T) std::atomic_ref<T>
#endif

#include <atomic>
#include <concepts>
#include <utility>

namespace vsm {
namespace detail {

template<typename T>
concept _atomic_ptr = std::is_pointer_v<T> && std::is_object_v<std::remove_pointer_t<T>>;

} // namespace detail

/// @brief Stripped down version of std::atomic_ref<@tparam T>.
/// All atomic operations are guaranteed to be lock free.
/// All atomic operations require explicit memory order specification.
/// The standard atomic operator overloads are not provided.
/// @tparam T Value type referred to by the atomic_ref.
template<typename T>
class atomic_ref : vsm_detail_atomic_ref_base(T)
{
	using base = vsm_detail_atomic_ref_base(T);

	static_assert(base::is_always_lock_free);

public:
	using base::is_always_lock_free;
	using base::required_alignment;


	using base::base;


	[[nodiscard]] constexpr bool is_lock_free() const noexcept
	{
		return true;
	}

	[[nodiscard]] vsm_always_inline T load(std::memory_order const order) const noexcept
	{
		return base::load(order);
	}

	vsm_always_inline void store(T const& new_value, std::memory_order const order) const noexcept
	{
		return base::store(new_value, order);
	}

	[[nodiscard]] vsm_always_inline T exchange(
		T const& new_value,
		std::memory_order const order) const noexcept
	{
		return base::exchange(new_value, order);
	}

	[[nodiscard]] vsm_always_inline bool compare_exchange_weak(
		T& expected,
		T const& desired,
		std::memory_order const success,
		std::memory_order const failure) const noexcept
	{
		return base::compare_exchange_weak(expected, desired, success, failure);
	}

	[[nodiscard]] vsm_always_inline bool compare_exchange_strong(
		T& expected,
		T const& desired,
		std::memory_order const success,
		std::memory_order const failure) const noexcept
	{
		return base::compare_exchange_strong(expected, desired, success, failure);
	}


#define vsm_detail_fetch_mutate(operation, type, constraint) \
	[[nodiscard]] vsm_always_inline decltype(auto) operation( \
		type const operand, \
		std::memory_order const order) const noexcept \
		requires constraint<T> \
	{ \
		return base::operation(operand, order); \
	} \

	vsm_detail_fetch_mutate(fetch_add, T, std::integral)
	vsm_detail_fetch_mutate(fetch_add, ptrdiff_t, detail::_atomic_ptr)
	vsm_detail_fetch_mutate(fetch_sub, T, std::integral)
	vsm_detail_fetch_mutate(fetch_sub, ptrdiff_t, detail::_atomic_ptr)
	vsm_detail_fetch_mutate(fetch_and, T, std::integral)
	vsm_detail_fetch_mutate(fetch_or, T, std::integral)
	vsm_detail_fetch_mutate(fetch_xor, T, std::integral)
#undef vsm_detail_fetch_mutate
};

template<typename T>
atomic_ref(T&) -> atomic_ref<T>;


/// @brief Stripped down version of std::atomic<@tparam T>.
/// All atomic operations are guaranteed to be lock free.
/// All atomic operations require explicit memory order specification.
/// The standard atomic operator overloads are not provided.
/// @tparam T Value type stored by the atomic object.
template<typename T>
class atomic
{
	using ref_type = atomic_ref<T>;

	alignas(ref_type::required_alignment) mutable T m_storage = {};

public:
	static constexpr bool is_always_lock_free = true;


	atomic()
		: m_storage{}
	{
	}

	atomic(T const value) noexcept
		requires std::is_fundamental_v<T>
		: m_storage(value)
	{
	}

	template<std::convertible_to<T> U = T>
	atomic(U&& value) noexcept
		requires (!std::is_fundamental_v<T>)
		: m_storage(vsm_forward(value))
	{
	}

	template<typename... Args>
	explicit atomic(std::in_place_t, Args&&... args)
		requires std::constructible_from<T, Args...>
		: m_storage(vsm_forward(args)...)
	{
	}

	atomic(atomic const&) = delete;
	atomic& operator=(atomic const&) = delete;


	[[nodiscard]] constexpr bool is_lock_free() const noexcept
	{
		return true;
	}

	[[nodiscard]] vsm_always_inline T load(std::memory_order const order) const noexcept
	{
		return ref_type(m_storage).load(order);
	}

	vsm_always_inline void store(T const& new_value, std::memory_order const order) & noexcept
	{
		return ref_type(m_storage).store(new_value, order);
	}

	[[nodiscard]] vsm_always_inline T exchange(
		T const& new_value,
		std::memory_order const order) & noexcept
	{
		return ref_type(m_storage).exchange(new_value, order);
	}

	[[nodiscard]] vsm_always_inline bool compare_exchange_weak(
		T& expected,
		T const& desired,
		std::memory_order const success,
		std::memory_order const failure) & noexcept
	{
		return ref_type(m_storage).compare_exchange_weak(expected, desired, success, failure);
	}

	[[nodiscard]] vsm_always_inline bool compare_exchange_strong(
		T& expected,
		T const& desired,
		std::memory_order const success,
		std::memory_order const failure) & noexcept
	{
		return ref_type(m_storage).compare_exchange_strong(expected, desired, success, failure);
	}


#define vsm_detail_fetch_mutate(operation, type, constraint) \
	[[nodiscard]] vsm_always_inline decltype(auto) operation( \
		type const operand, \
		std::memory_order const order) & noexcept \
		requires constraint<T> \
	{ \
		return ref_type(m_storage).operation(operand, order); \
	} \

	vsm_detail_fetch_mutate(fetch_add, T, std::integral)
	vsm_detail_fetch_mutate(fetch_add, ptrdiff_t, detail::_atomic_ptr)
	vsm_detail_fetch_mutate(fetch_sub, T, std::integral)
	vsm_detail_fetch_mutate(fetch_sub, ptrdiff_t, detail::_atomic_ptr)
	vsm_detail_fetch_mutate(fetch_and, T, std::integral)
	vsm_detail_fetch_mutate(fetch_or, T, std::integral)
	vsm_detail_fetch_mutate(fetch_xor, T, std::integral)
#undef vsm_detail_fetch_mutate


	operator ref_type() &
	{
		return ref_type(m_storage);
	}
};

} // namespace vsm
