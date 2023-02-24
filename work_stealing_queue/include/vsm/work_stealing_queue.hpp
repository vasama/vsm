#pragma once

#include <vsm/bounded_work_stealing_queue.hpp>
#include <vsm/default_allocator.hpp>
#include <vsm/intrusive_ptr.hpp>

namespace vsm {

template<typename T, typename Allocator = default_allocator>
class work_stealing_thief;

template<typename T, typename Allocator = default_allocator>
class work_stealing_queue
{
	static_assert(std::is_trivial_v<T>);

	struct queue_block : intrusive_ref_count
	{
		size_t const m_size;
		queue_block* m_small;

		atomic<queue_block*> m_large = nullptr;
		detail::work_stealing_queue_control m_control;
	
		T m_data[];


		explicit queue_block(size_t const size)
			: m_size(size)
			, m_small(nullptr)
		{
			vsm_assert(is_power_of_two(size));
			std::uninitialized_default_construct_n(m_data, size);
		}

		explicit queue_block(size_t const size, queue_block* const small, std::span<T const> const data)
			: m_size(size)
			, m_small(small)
			, m_control(data.size())
		{
			vsm_assert(is_power_of_two(size));
			vsm_assert(data.size() <= size);

			std::uninitialized_default_construct(
				std::uninitialized_copy(data.data(), data.size(), m_data),
				m_data + size);
		}

		queue_block(queue_block const&) = delete;
		queue_block& operator=(queue_block const&) = delete;


		size_t push_some(std::span<T const> const data)
		{
			return m_control.push_some(std::span<T>(m_data, m_size), data);
		}
		
		size_t pop_some(std::span<T> const out_data)
		{
			return m_control.pop_some(std::span<T>(m_data, m_size), out_data);
		}
		
		size_t steal_some(std::span<T> const out_data)
		{
			return m_control.steal_some(std::span<T>(m_data, m_size), out_data);
		}
	};

	using queue_block_ptr = intrusive_ptr<queue_block>;

	queue_block* m_stack_top;
	queue_block_ptr m_stack_bottom;

public:
	work_stealing_queue(size_t const min_initial_size)
		: m_stack_top(make_block(round_up_to_power_of_two(min_initial_size)))
		, m_stack_bottom(m_stack_top)
	{
	}

	work_stealing_queue(work_stealing_queue const&) = delete;
	work_stealing_queue& operator=(work_stealing_queue const&) = delete;


	void push_all(std::span<T const> const data)
	{
		size_t const push_count = m_stack_top->push_some(data);
		
		if (push_count != data.size())
		{
			push_all_slow(data.subspan(push_count));
		}
	}

	void push_one(T const& data)
	{
		push_all(std::span<T const>(&data, 1));
	}

	size_t pop_some(std::span<T> const out_data)
	{
		if (size_t const count = m_stack_top->pop_some(out_data))
		{
			return count;
		}
		
		return pop_some_slow(out_data);
	}

	bool pop_one(T& out_value)
	{
		return pop_some(std::span<T>(&out_value, 1)) != 0;
	}

	std::optional<T> pop_one()
	{
		std::optional<T> optional(std::in_place);
		if (pop_some(std::span<T>(&*optional, 1)) == 0)
		{
			optional.reset();
		}
		return optional;
	}


	work_stealing_thief<T, Allocator> get_thief()
	{
		return work_stealing_thief<T, Allocator>(m_stack_bottom);
	}

private:
	void push_all_slow(std::span<T const> const data)
	{
		vsm_assert(!data.empty());
	
		queue_block* const top = m_stack_top.get();
		
		size_t const min_size = round_up_to_power_of_two(data.size());
		size_t const new_size = std::max(min_size, top->m_size * 2);
		
		queue_block* const new_top = make_block(new_size, data);
		top->m_large.store(new_top, std::memory_order_release);
		m_stack_top = new_top;
	}
	
	size_t pop_some_slow(std::span<T> const out_data)
	{
		vsm_assert(false && "not implemented");
	}
	
	static queue_block* make_block(size_t const size, auto&&... args)
	{
		static constexpr size_t header_size = offsetof(queue_block, m_data);
		void* const storage = operator new(header_size + size * sizeof(T));
		return new (storage) queue_block(size, vsm_forward(args)...);
	}

	friend class work_stealing_thief<T, Allocator>;
};

template<typename T, typename Allocator>
class work_stealing_thief
{
	using queue_type = work_stealing_queue<T, Allocator>;
	using queue_block = typename queue_type::queue_block;
	using queue_block_ptr = typename queue_type::queue_block_ptr;

	queue_block_ptr m_bottom;

public:
	bool steal_one(T& out_value)
	{
		queue_block* bottom = m_bottom.get();
		
		while (true)
		{
			// Attempt to steal some from the bottom block.
			// If at least one value was available, we're done here.
			if (bottom->steal_one(out_value))
			{
				return true;
			}

			// Load the next block moving up towards the top of the stack.
			bottom = bottom->large.load(std::memory_order_acquire);
			
			// If we have reached the top of the stack, we're done here.
			if (bottom == nullptr)
			{
				return false;
			}

			// The old bottom block will never get new data,
			// so we can release our shared pointer to it.
			m_bottom.reset(bottom);
		}
	}
	
	std::optional<T> steal_one()
	{
		std::optional<T> optional(std::in_place);
		if (!steal_one(*optional))
		{
			optional.reset();
		}
		return optional;
	}

private:
	explicit thief(queue_block_ptr bottom)
		: m_bottom(vsm_move(bottom))
	{
	}

	friend class work_stealing_queue<T, Allocator>;
};

} // namespace vsm
