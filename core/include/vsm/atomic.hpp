#pragma once

#include <vsm/detail/categories.hpp>
#include <vsm/platform.h>
#include <vsm/type_traits.hpp>
#include <vsm/utility.hpp>

#if vsm_arch_x86_64
#	include <vsm/detail/atomic_x86_64.hpp>

#	define vsm_detail_atomic_ref_base(T) \
		select_t<sizeof(T) == 16, detail::atomic_ref_x86_64<T>, std::atomic_ref<T>>
#else
#	define vsm_detail_atomic_ref_base(T) std::atomic_ref<T>
#endif

#include <atomic>
#include <concepts>

namespace vsm {
#if 0
namespace detail {

template<template<typename> typename Storage, typename T>
	requires std::atomic_ref<T>::is_always_lock_free
class atomic_wrapper
{
	using ref_type = std::atomic_ref<T>;

	using cast_type = select_t<
		std::is_same_v<Storage<T>, T>,
		ref_type,
		ref_type const&>;

	mutable Storage<T> m_storage = {};

public:
	atomic_wrapper() = default;

	vsm_always_inline atomic_wrapper(std::convertible_to<Storage<T>> auto&& value)
		: m_storage(vsm_forward(value))
	{
	}


	[[nodiscard]] vsm_always_inline T load(std::memory_order const order) const
	{
		return static_cast<cast_type>(m_storage).load(order);
	}

	vsm_always_inline void store(T const& new_value, std::memory_order const order) &
	{
		return static_cast<cast_type>(m_storage).store(new_value, order);
	}

	[[nodiscard]] vsm_always_inline T exchange(T const& new_value, std::memory_order const order) &
	{
		return static_cast<cast_type>(m_storage).exchange(new_value, order);
	}

	[[nodiscard]] vsm_always_inline bool compare_exchange_weak(
		T& expected, T const& desired, std::memory_order const success, std::memory_order const failure) &
	{
		return static_cast<cast_type>(m_storage).compare_exchange_weak(expected, desired, success, failure);
	}

	[[nodiscard]] vsm_always_inline bool compare_exchange_strong(
		T& expected, T const& desired, std::memory_order const success, std::memory_order const failure) &
	{
		return static_cast<cast_type>(m_storage).compare_exchange_strong(expected, desired, success, failure);
	}

#define vsm_detail_fetch_mutate(operation) \
	vsm_always_inline auto operation(auto const& operand, std::memory_order const order) & \
		-> decltype(std::declval<ref_type const&>().operation(operand, order)) \
	{ \
		return static_cast<cast_type>(m_storage).operation(operand, order); \
	} \

	vsm_detail_fetch_mutate(fetch_add)
	vsm_detail_fetch_mutate(fetch_sub)
	vsm_detail_fetch_mutate(fetch_and)
	vsm_detail_fetch_mutate(fetch_or)
	vsm_detail_fetch_mutate(fetch_xor)
#undef vsm_detail_fetch_mutate
};

template<typename T>
using atomic_storage = T;

template<typename T>
using atomic_ref_storage = T*;

} // namespace detail

/// @brief Stripped down version of std::atomic<@tparam T>.
/// All atomic operations are guaranteed to be lock free.
/// All atomic operations require explicit memory order specification.
/// The standard atomic operator overloads are not provided.
/// @tparam T Value type stored by the atomic object.
template<typename T>
using atomic = detail::atomic_wrapper<detail::atomic_storage, T>;

/// @brief Stripped down version of std::atomic_ref<@tparam T>.
/// All atomic operations are guaranteed to be lock free.
/// All atomic operations require explicit memory order specification.
/// The standard atomic operator overloads are not provided.
/// @tparam T Value type referred to by the atomic object.
template<typename T>
using atomic_ref = detail::atomic_wrapper<detail::atomic_ref_storage, T>;
#endif


/// @brief Stripped down version of std::atomic_ref<@tparam T>.
/// All atomic operations are guaranteed to be lock free.
/// All atomic operations require explicit memory order specification.
/// The standard atomic operator overloads are not provided.
/// @tparam T Value type referred to by the atomic object.
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

	[[nodiscard]] vsm_always_inline T exchange(T const& new_value, std::memory_order const order) const noexcept
	{
		return base::exchange(new_value, order);
	}

	[[nodiscard]] vsm_always_inline bool compare_exchange_weak(
		T& expected, T const& desired, std::memory_order const success, std::memory_order const failure) const noexcept
	{
		return base::compare_exchange_weak(expected, desired, success, failure);
	}

	[[nodiscard]] vsm_always_inline bool compare_exchange_strong(
		T& expected, T const& desired, std::memory_order const success, std::memory_order const failure) const noexcept
	{
		return base::compare_exchange_strong(expected, desired, success, failure);
	}


#define vsm_detail_fetch_mutate(operation) \
	[[nodiscard]] vsm_always_inline auto operation(auto const& operand, std::memory_order const order) const noexcept \
		-> decltype(base::operation(operand, order)) \
	{ \
		return base::operation(operand, order); \
	} \

	vsm_detail_fetch_mutate(fetch_add)
	vsm_detail_fetch_mutate(fetch_sub)
	vsm_detail_fetch_mutate(fetch_and)
	vsm_detail_fetch_mutate(fetch_or)
	vsm_detail_fetch_mutate(fetch_xor)
#undef vsm_detail_fetch_mutate
};

/// @brief Stripped down version of std::atomic<@tparam T>.
/// All atomic operations are guaranteed to be lock free.
/// All atomic operations require explicit memory order specification.
/// The standard atomic operator overloads are not provided.
/// @tparam T Value type stored by the atomic object.
template<typename T>
class atomic
{
	using ref_type = atomic_ref<T>;

	mutable alignas(ref_type::required_alignment) T m_storage = {};

public:
	static constexpr bool is_always_lock_free = true;


	atomic() = default;

	template<std::convertible_to<T> U = T>
	atomic(U&& value) noexcept
		: m_storage(vsm_forward(value))
	{
	}


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

	[[nodiscard]] vsm_always_inline T exchange(T const& new_value, std::memory_order const order) & noexcept
	{
		return ref_type(m_storage).exchange(new_value, order);
	}

	[[nodiscard]] vsm_always_inline bool compare_exchange_weak(
		T& expected, T const& desired, std::memory_order const success, std::memory_order const failure) & noexcept
	{
		return ref_type(m_storage).compare_exchange_weak(expected, desired, success, failure);
	}

	[[nodiscard]] vsm_always_inline bool compare_exchange_strong(
		T& expected, T const& desired, std::memory_order const success, std::memory_order const failure) & noexcept
	{
		return ref_type(m_storage).compare_exchange_strong(expected, desired, success, failure);
	}


#define vsm_detail_fetch_mutate(operation) \
	[[nodiscard]] vsm_always_inline auto operation(auto const& operand, std::memory_order const order) & noexcept \
		-> decltype(ref_type(m_storage).operation(operand, order)) \
	{ \
		return ref_type(m_storage).operation(operand, order); \
	} \

	vsm_detail_fetch_mutate(fetch_add)
	vsm_detail_fetch_mutate(fetch_sub)
	vsm_detail_fetch_mutate(fetch_and)
	vsm_detail_fetch_mutate(fetch_or)
	vsm_detail_fetch_mutate(fetch_xor)
#undef vsm_detail_fetch_mutate
};

} // namespace vsm
