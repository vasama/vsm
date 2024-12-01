#pragma once

#include <vsm/assert.h>
#include <vsm/atomic.hpp>
#include <vsm/type_traits.hpp>

#include <bit>
#include <span>

#include <cstddef>

namespace vsm {
namespace detail {

//TODO: Optimize for random access/contiguous iterators
template<typename SrcIterator, typename SrcSentinel, typename OutIterator>
OutIterator ring_copy(
	SrcIterator src_beg,
	SrcSentinel const src_end,
	OutIterator const out_beg,
	OutIterator out_pos,
	OutIterator const out_end)
{
	for (; src_beg != src_end; (void)++src_beg, (void)++out_pos)
	{
		if (out_pos == out_end)
		{
			out_pos = out_beg;
		}
		*out_pos = *src_beg;
	}
	return out_pos;
}

template<typename T>
class _work_stealing_queue
{
	// https://www.dre.vanderbilt.edu/~schmidt/PDF/work-stealing-dequeue.pdf
	// https://fzn.fr/readings/ppopp13.pdf

	using offset_type = uint64_t;

	atomic<offset_type> m_owner_offset = 1;
	atomic<offset_type> m_thief_offset = 1;

public:
	[[nodiscard]] size_t push_some(std::span<T> const array, std::span<T const> const data)
	{
		size_t const size = array.size();
		vsm_assert(std::has_single_bit(size));
		size_t const mask = size - 1;

		offset_type const owner_offset = m_owner_offset.load(std::memory_order_relaxed);
		offset_type const thief_offset = m_thief_offset.load(std::memory_order_acquire);
		vsm_assert_slow(thief_offset <= owner_offset);

		size_t const space = size - static_cast<size_t>(owner_offset - thief_offset);

		if (space == 0)
		{
			return 0;
		}

		size_t const count = std::min(data.size(), space);

		T const* const data_beg = data.data();
		T const* const data_end = data_beg + count;

		T* const ring_beg = array.data();
		T* const ring_pos = ring_beg + (owner_offset & mask);
		T* const ring_end = ring_beg + array.size();

		ring_copy(
			data_beg, data_end,
			ring_beg, ring_pos, ring_end);

		m_owner_offset.store(owner_offset + count, std::memory_order_release);

		return count;
	}

	[[nodiscard]] bool take_one(std::span<T> const array, T& out)
	{
		size_t const size = array.size();
		vsm_assert(std::has_single_bit(size));
		size_t const mask = size - 1;

		offset_type const owner_offset = m_owner_offset.load(std::memory_order_relaxed) - 1;
		m_owner_offset.store(owner_offset, std::memory_order_relaxed);
		std::atomic_thread_fence(std::memory_order_seq_cst);

		offset_type const thief_offset = m_thief_offset.load(std::memory_order_relaxed);

		if (thief_offset > owner_offset)
		{
			m_owner_offset.store(owner_offset + 1, std::memory_order_relaxed);
			return false;
		}

		if (thief_offset == owner_offset)
		{
			offset_type expected_thief_offset = thief_offset;
			bool const result = m_thief_offset.compare_exchange_strong(
				expected_thief_offset,
				thief_offset + 1,
				std::memory_order_seq_cst,
				std::memory_order_relaxed);

			m_owner_offset.store(owner_offset + 1, std::memory_order_relaxed);

			if (!result)
			{
				return false;
			}
		}

		out = array[owner_offset & mask];

		return true;
	}

	[[nodiscard]] bool steal_one(std::span<T> const array, T& out)
	{
		size_t const size = array.size();
		vsm_assert(std::has_single_bit(size));
		size_t const mask = size - 1;

		offset_type const thief_offset = m_thief_offset.load(std::memory_order_acquire);
		std::atomic_thread_fence(std::memory_order_seq_cst);
		offset_type const owner_offset = m_owner_offset.load(std::memory_order_acquire);

		if (thief_offset >= owner_offset)
		{
			return false;
		}

		// It does not matter if this operation tears. That can only happen if thief_offset was
		// changed by another thread, in which case the subsequent compare-exchange will fail and
		// whatever result was read will be disregarded.
		out = array[thief_offset & mask];

		offset_type expected_thief_offset = thief_offset;
		return m_thief_offset.compare_exchange_weak(
			expected_thief_offset,
			thief_offset + 1,
			std::memory_order_seq_cst,
			std::memory_order_relaxed);
	}
};

} // namespace detail

template<typename T>
class bounded_work_stealing_queue
{
	static_assert(std::is_trivial_v<T>);

	detail::_work_stealing_queue<T> m_queue;

public:
	
};

} // namespace vsm
