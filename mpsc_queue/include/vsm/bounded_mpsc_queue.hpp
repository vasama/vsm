#pragma once

#include <vsm/atomic.hpp>
#include <vsm/math.hpp>

#include <span>

namespace vsm {
namespace detail {

class _mpsc_queue
{
	atomic<size_t> m_size = 0;
	atomic<size_t> m_head = 0;
	size_t m_tail = 0;

public:
	template<typename T>
	bool try_push(std::span<T> const queue, T const& value, T const& null_value)
	{
		vsm_assert(value != null_value);
		vsm_assert(is_power_of_two(queue.size()));
		size_t const mask = queue.size() - 1;

		if (m_size.fetch_add(1, std::memory_order_acquire) > mask)
		{
			m_size.fetch_sub(1, std::memory_order_release);
			return false;
		}
		
		size_t const head = m_head.fetch_add(1, std::memory_order_acq_rel);
		
		auto slot = atomic_ref<T>(queue[head & mask]);
		vsm_assert(slot.load(std::memory_order_relaxed) == null_value);
		slot.store(value, std::memory_order_release);
		
		return true;
	}
	
	template<typename T>
	bool try_pop(std::span<T> const queue, T& out_value, T const& null_value)
	{
		vsm_assert(is_power_of_two(queue.size()));
		size_t const mask = queue.size() - 1;
		
		size_t const tail = m_tail;

		auto slot = atomic_ref<T>(queue[tail & mask]);
		T const value = slot.load(std::memory_order_acquire);
		
		if (value == null_value)
		{
			return false;
		}
		
		slot.store(null_value, std::memory_order_release);
		
		m_tail = m_tail + 1;
		out_value = value;

		return true;
	}
};

} // namespace detail

template<typename T>
class bounded_mpsc_queue
{
	detail::_mpsc_queue m_control;
};

} // namespace vsm
