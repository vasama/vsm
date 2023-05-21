#pragma once

#include <vsm/detail/categories.hpp>
#include <vsm/platform.h>
#include <vsm/type_traits.hpp>
#include <vsm/utility.hpp>

#include <atomic>
#include <concepts>

namespace vsm {

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
using atomic_ref_storage = std::atomic_ref<T>;

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

} // namespace vsm
