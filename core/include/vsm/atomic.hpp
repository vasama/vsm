#pragma once

#include <vsm/platform.h>

#include <atomic>

namespace vsm {

namespace detail {

template<template<typename> typename Atomic, typename T>
	requires Atomic<T>::is_always_lock_free
class atomic_wrapper : Atomic<T>
{
public:
	using Atomic<T>::Atomic;

	vsm_always_inline T load(std::memory_order const order) const
	{
		return Atomic<T>::load(order);
	}

	vsm_always_inline void store(T const& new_value, std::memory_order const order)
	{
		return Atomic<T>::store(new_value, order);
	}
	
	vsm_always_inline T exchange(T const& new_value, std::memory_order const order)
	{
		return Atomic<T>::exchange(new_value, order);
	}
	
	vsm_always_inline T compare_exchange_weak(
		T& expected, T const& desired,
		std::memory_order const success, std::memory_order const failure)
	{
		return Atomic<T>::compare_exchange_weak(expected, desired, success, failure);
	}

	vsm_always_inline T compare_exchange_strong(
		T& expected, T const& desired,
		std::memory_order const success, std::memory_order const failure)
	{
		return Atomic<T>::compare_exchange_strong(expected, desired, success, failure);
	}

#define vsm_detail_fetch_mutate(operation) \
	vsm_always_inline auto operation(auto const& operand, std::memory_order const order) \
		-> decltype(Atomic<T>::operation(operand, order)) \
	{ \
		return Atomic<T>::operation(operand, order); \
	}
	
	vsm_detail_fetch_mutate(fetch_add)
	vsm_detail_fetch_mutate(fetch_sub)
	vsm_detail_fetch_mutate(fetch_and)
	vsm_detail_fetch_mutate(fetch_or)
	vsm_detail_fetch_mutate(fetch_xor)
#undef vsm_detail_fetch_mutate

	Atomic<T>& get_std_atomic()
	{
		return *this;
	}
};

} // namespace detail

template<typename T>
using atomic = detail::atomic_wrapper<std::atomic, T>;

template<typename T>
using atomic_ref = detail::atomic_wrapper<std::atomic_ref, T>;


inline vsm_always_inline void atomic_spin_hint()
{
	vsm_atomic_spin_hint();
}

} // namespace vsm
