#pragma once

#include <vsm/default_allocator.hpp>

#include <cstddef>

namespace vsm {

template<typename Time, typename Key, typename Value, typename Allocator = default_allocator>
class timer_queue
{
	static_assert(std::is_trivially_copyable_v<Time>);

	using size_type = uint32_t;

	struct heap_slot
	{
		Time time;
		size_type data_index;
	};
	
	struct data_slot
	{
		size_type heap_index;
		Value value;
	};

	heap_slot* m_heap;
	data_slot* m_data;

	size_type m_size = 0;
	size_type m_capacity = 0;
	size_type m_freelist = static_cast<size_type>(-1);

	[[no_unique_address]] Allocator m_allocator;

public:
	[[nodiscard]] bool empty() const
	{
		return m_size == 0;
	}

	[[nodiscard]] std::tuple<Time const&, Value&> front()
	{
		return { m_heap->time, m_data[m_heap->data_index].value };
	}

	[[nodiscard]] std::tuple<Time const&, Value const&> front() const
	{
		return { m_heap->time, m_data[m_heap->data_index].value };
	}


	template<std::convertible_to<Value> InValue = Value>
	Key schedule(Time const time, InValue&& value)
	{
		size_type const data_index = acquire_slot();
		data_slot& data = m_data[data_index];

		heap_push(time, data_index, vsm_forward(value));

		vsm_assert_slow(check_heap());
		vsm_assert_slow(check_data());

		return static_cast<Key>(data_index);
	}

	void cancel(Key const key)
	{
		size_type const data_index = static_cast<size_type>(key);
		vsm_assert(data_index < m_capacity);

		heap_erase(m_data[data_index].heap_index);
		release_slot(data_index);

		vsm_assert_slow(check_heap());
		vsm_assert_slow(check_data());
	}

	void expire_all(Time const until, auto&& consumer)
	{
		while (m_size != 0)
		{
			heap_slot const& heap = *m_heap;

			if (until < heap.time)
			{
				break;
			}
			
			size_type const data_index = heap.data_index;
			vsm_assert(data_index < m_capacity);
			data_slot& data = m_data[data_index];

			Value value = vsm_move(data.value);

			heap_pop();
			release_slot(data_index);

			vsm_assert_slow(check_heap());
			vsm_assert_slow(check_data());

			callback(vsm_move(value));
		}
	}

private:
	static constexpr bool allocation_flip = alignof(heap_slot) < alignof(data_slot);

	size_type acquire_slot()
	{
		if (m_size == m_capacity)
		{
			return acquire_slot_slow();
		}

		size_type const index = m_freelist;
		m_freelist = m_data[index].heap_index;
		return index;
	}

	size_t acquire_slot_slow()
	{
		static constexpr size_t slot_size = sizeof(heap_slot) + sizeof(data_slot);
		static constexpr size_t slot_align = std::max(alignof(heap_slot), alignof(data_slot));

		heap_slot* const old_heap = m_heap;
		data_slot* const old_data = m_data;

		size_type const old_size = m_size;
		size_type const old_capacity = m_capacity;

		size_type new_capacity = old_capacity * 3 / 2;

		auto const allocation = m_allocator.allocate(std::min(1zu, new_capacity * slot_size));
		new_capacity = allocation.size / slot_size;

		heap_slot* const new_heap = reinterpret_cast<heap_slot*>(
			reinterpret_cast<std::byte*>(allocation.data) + (allocation_flip ? old_capacity * sizeof(data_slot) : 0));

		data_slot* const new_data = reinterpret_cast<data_slot*>(
			reinterpret_cast<std::byte*>(allocation.data) + (allocation_flip ? 0 : old_capacity * sizeof(heap_slot)));

		std::uninitialized_copy_n(new_heap, old_heap, old_size);

		for (heap_slot const& h : std::span(old_heap, old_size))
		{
			new (new_data + h.data_index) data_slot(old_data[h.data_index]);
		}

		// Copy slot freelist.
		{
			size_type list_index = m_freelist;

			for (size_type size = old_size; size < old_capacity; ++size)
			{
				size_type const next_index = old_data[list_index];
				new_data[list_index] = next_index;
				list_index = next_index;
			}

			for (size_type size = old_capacity; size < new_capacity; ++size)
			{
				new_data[size] = size + 1;
			}
		}

		m_allocator.deallocate(allocation{ allocation_flip ? old_data : old_heap, old_capacity * slot_size });
	}

	void release_slot(size_type const index)
	{
		m_data[index].heap_index = m_freelist;
		m_freelist = index;
	}


	bool check_heap() const
	{
		return std::is_heap(m_heap, m_heap + m_size, [](heap_slot const& a, heap_slot const& b)
		{
			return a.time > b.time;
		});
	}

	bool check_data() const
	{
		return true;
	}
};

} // namespace vsm
