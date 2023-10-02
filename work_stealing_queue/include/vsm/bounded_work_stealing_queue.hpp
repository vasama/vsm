#pragma once

#include <vsm/atomic.hpp>
#include <vsm/type_traits.hpp>

#include <span>

#include <cstddef>

namespace vsm {
namespace detail {

class work_stealing_queue_control
{
	atomic<size_t> m_head = 0;
	atomic<size_t> m_tail = 0;

public:
	work_stealing_queue_control() = default;

	explicit work_stealing_queue_control(size_t const initial_data_count)
		: m_head(initial_data_count)
	{
	}


	template<typename T>
	size_t push_some(std::span<T> const queue, std::span<T const> const data)
	{
		size_t const tail = m_tail.load(std::memory_order_relaxed);
		size_t const head = m_head.load(std::memory_order_relaxed);

		// If the queue is full.
		if ((head & mask) == tail && head > tail)
		{
			return 0;
		}

		// Insert work at the head.
		queue[head & mask] = value;

		
	}

	template<typename T>
	size_t pop_some(std::span<T> const queue, std::span<T> const out_data)
	{
	}

	template<typename T>
	size_t steal_some(std::span<T> const queue, std::span<T> const out_data)
	{
	}
};

} // namespace detail

template<typename T>
class bounded_work_stealing_queue
{
	static_assert(std::is_trivial_v<T>);

	detail::work_stealing_queue_control<T> m_control;

public:
	
};

} // namespace vsm
