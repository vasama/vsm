#pragma once

#include <vsm/bounded_mpsc_queue.hpp>
#include <vsm/default_allocator.hpp>
#include <vsm/intrusive_ptr.hpp>

namespace vsm {

template<typename P, typename Allocator = default_allocator>
class mpsc_queue_consumer;

template<typename P, typename Allocator = default_allocator>
class mpsc_queue_producer;

template<typename T, typename Allocator>
class mpsc_queue_consumer<T*, Allocator>
{
	struct queue_block : intrusive_ref_count
	{
		using queue_block_ptr = intrusive_ptr<queue_block>;

		atomic<queue_block*> m_next;
		detail::_mpsc_queue m_queue;

		size_t const m_size;
		T* m_data[];


		explicit queue_block(size_t const size)
			: m_size(size)
		{
			std::uninitialized_default_construct_n(m_data, size);
		}

		queue_block(queue_block const&) = delete;
		queue_block& operator=(queue_block const&) = delete;

		~queue_block()
		{
			queue_block_ptr::acquire(m_next.load(std::memory_order_relaxed));
		}


		queue_block* get_next() const
		{
			return m_next.load(std::memory_order_acquire);
		}

		queue_block* set_or_get_next(queue_block_ptr&& next)
		{
			queue_block* desired = nullptr;
			if (m_next.compare_exchange_strong(
				desired, next.get(),
				std::memory_order_release, std::memory_order_relaxed))
			{
				(void)next.release();
			}
			return desired;
		}


		bool push_one(T* const ptr)
		{
			return m_queue.try_push(std::span<T>(m_data, m_size), ptr, nullptr);
		}

		T* pop_one()
		{
			T* ptr;
			return m_queue.try_pop(std::span<T>(m_data, m_size), ptr, nullptr)
				? ptr
				: nullptr;
		}
	};
	using queue_block_ptr = intrusive_ptr<queue_block>;

	queue_block_ptr m_bottom;
	Allocator m_allocator;

public:
	explicit mpsc_queue_consumer(size_t const initial_capacity)
		: m_bottom(make_block(round_up_to_power_of_two(initial_capacity)))
	{
	}

	mpsc_queue_consumer(mpsc_queue_consumer&&) = default;
	mpsc_queue_consumer& operator=(mpsc_queue_consumer&&) = default;


	[[nodiscard]] T* pop_one()
	{
		if (T* const ptr = m_bottom->pop_one())
		{
			return ptr;
		}

		return pop_one_slow();
	}

	[[nodiscard]] mpsc_queue_producer<T, Allocator> get_producer()
	{
		return mpsc_queue_producer<T, Allocator>(m_bottom, m_allocator);
	}

private:
	T* pop_one_slow()
	{
		while (true)
		{
			queue_block* const next = m_bottom->get_next();

			if (next == nullptr)
			{
				return nullptr;
			}

			m_bottom = next;

			if (T* const ptr = next->pop_one())
			{
				return ptr;
			}
		}
	}

	static queue_block_ptr make_block(size_t const size, auto&&... args)
	{
		static constexpr size_t header_size = offsetof(queue_block, m_data);
		void* const storage = operator new(header_size + size * sizeof(T*));
		return queue_block_ptr(new (storage) queue_block(size, vsm_forward(args)...));
	}

	friend class mpsc_queue_producer<T, Allocator>;
};

template<typename T, typename Allocator>
class mpsc_queue_producer<T*, Allocator>
{
	using consumer_type = mpsc_queue_consumer<T, Allocator>;
	using queue_block = typename consumer_type::queue_block;
	using queue_block_ptr = typename consumer_type::queue_block_ptr;

	queue_block_ptr m_top;

public:
	explicit mpsc_queue_producer(queue_block_ptr top)
		: m_top(vsm_move(top))
	{
	}


	void push_one(T* const ptr)
	{
		if (!m_top->push_one(ptr))
		{
			return push_one_slow(ptr);
		}
	}

private:
	void push_all_slow(std::span<T* const> const data)
	{
		vsm_assert(!data.empty());
		vsm_assert(false);

		size_t const min_size = round_up_to_power_of_two(data.size());

		for (queue_block* top = m_top.get();;)
		{
			size_t const new_size = std::max(min_size, top->m_size * 2);

			queue_block_ptr new_top = consumer_type::make_block(new_size, data);
			top = top->get_or_set_next(vsm_move(new_top));

			if (top != nullptr)
			{
			}
		}
	}

	friend class mpsc_queue_consumer<T, Allocator>;
};

} // namespace vsm
