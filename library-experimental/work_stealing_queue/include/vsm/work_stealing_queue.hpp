#pragma once

#include <vsm/bounded_work_stealing_queue.hpp>
#include <vsm/default_allocator.hpp>
#include <vsm/intrusive_ptr.hpp>
#include <vsm/math.hpp>
#include <vsm/standard.hpp>

#include <optional>

namespace vsm {

template<typename T, typename Allocator = default_allocator>
class work_stealing_thief;

template<typename T, typename Allocator = default_allocator>
class work_stealing_queue
{
	static_assert(std::is_trivially_copyable_v<T>);

	struct queue_block : intrusive_refcount
	{
		size_t const m_size;
		queue_block* m_prev_block;

		atomic<queue_block*> m_next_block = nullptr;
		detail::_work_stealing_queue<T> m_queue;

		T m_data[];

		explicit queue_block(size_t const size, queue_block* const prev = nullptr)
			: m_size(size)
			, m_prev_block(prev)
			, m_data{}
		{
			vsm_assert(std::has_single_bit(size));
			vsm::start_lifetime_as_array<T>(m_data, size);
		}

		explicit queue_block(size_t const size, queue_block* const prev, std::span<T const> const data)
			: queue_block(size, prev)
		{
			vsm_assert(size >= data.size());
			vsm_verify(m_queue.push_some(std::span(m_data, size), data) == data.size());
		}

		queue_block(queue_block const&) = delete;
		queue_block& operator=(queue_block const&) = delete;


		[[nodiscard]] size_t push_some(std::span<T const> const data)
		{
			return m_queue.push_some(std::span<T>(m_data, m_size), data);
		}

		[[nodiscard]] bool take_one(T& out)
		{
			return m_queue.take_one(std::span<T>(m_data, m_size), out);
		}

		[[nodiscard]] bool steal_one(T& out)
		{
			return m_queue.steal_one(std::span<T>(m_data, m_size), out);
		}
	};

	using queue_block_manager = basic_refcount_manager<Allocator>;
	using queue_block_ptr = intrusive_ptr<queue_block, queue_block_manager>;

	queue_block_ptr m_stack_bottom;
	queue_block* m_stack_top;

public:
	explicit work_stealing_queue(
		size_t const min_initial_size,
		Allocator const& allocator = Allocator())
		: m_stack_bottom(make_block(allocator, std::bit_ceil(min_initial_size)))
		, m_stack_top(m_stack_bottom.get())
	{
	}

	work_stealing_queue(work_stealing_queue&& other) noexcept
		: m_stack_bottom(vsm_move(other.m_stack_bottom))
		, m_stack_top(std::exchange(other.m_stack_top, nullptr))
	{
	}

	work_stealing_queue& operator=(work_stealing_queue&& other) & noexcept
	{
		m_stack_bottom = vsm_move(other.m_stack_bottom);
		m_stack_top = std::exchange(other.m_stack_top, nullptr);
		return *this;
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

	[[nodiscard]] bool take_one(T& out_value)
	{
		for (queue_block* block = m_stack_top; block != nullptr; block = block->m_prev_block)
		{
			if (block->take_one(out_value))
			{
				return true;
			}
		}

		if (m_stack_top != m_stack_bottom)
		{
			// The blocks below the top will no longer be used. Any new elements will always be
			// pushed into the current top block, or into a new future top block. Thieves only move
			// towards the top and never touch queue_block::m_prev_block.
			m_stack_bottom.reset(m_stack_top);
			m_stack_top->m_prev_block = nullptr;
		}

		return false;
	}

	[[nodiscard]] std::optional<T> take_one()
	{
		std::optional<T> optional(std::in_place);
		if (!take_one(*optional))
		{
			optional.reset();
		}
		return optional;
	}


	[[nodiscard]] Allocator const& get_allocator() const
	{
		return m_stack_bottom.get_manager().get_allocator();
	}

	[[nodiscard]] work_stealing_thief<T, Allocator> get_thief()
	{
		return work_stealing_thief<T, Allocator>(m_stack_bottom);
	}

private:
	void push_all_slow(std::span<T const> const data)
	{
		vsm_assert(!data.empty());

		queue_block* const top = m_stack_top;

		size_t const min_size = std::bit_ceil(data.size());
		size_t const new_size = std::max(min_size, top->m_size * 2);

		queue_block_ptr new_top = make_block(get_allocator(), new_size, top, data);
		top->m_next_block.store(new_top.get(), std::memory_order_release);
		m_stack_top = new_top.release();
	}

	static queue_block_ptr make_block(Allocator const& allocator, size_t const size, auto&&... args)
	{
		static constexpr size_t header_size = offsetof(queue_block, m_data);
		void* const storage = vsm::allocate_or_throw(allocator, header_size + size * sizeof(T));
		queue_block* const block = new (storage) queue_block(size, vsm_forward(args)...);
		return queue_block_ptr(block, queue_block_manager(allocator));
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
	[[nodiscard]] bool steal_one(T& out_value)
	{
		queue_block* bottom = m_bottom.get();
		
		while (true)
		{
			// Attempt to steal some from the bottom block. If at least one value was available,
			// we're done here.
			if (bottom->steal_one(out_value))
			{
				return true;
			}

			// Load the next block moving up towards the top of the stack.
			bottom = bottom->m_next_block.load(std::memory_order_acquire);
			
			// If we have reached the top of the stack, we're done here.
			if (bottom == nullptr)
			{
				return false;
			}

			// The old bottom block will never get new data, so we can release our shared pointer to
			// it.
			m_bottom.reset(bottom);
		}
	}

	[[nodiscard]] std::optional<T> steal_one()
	{
		std::optional<T> optional(std::in_place);
		if (!steal_one(*optional))
		{
			optional.reset();
		}
		return optional;
	}

private:
	explicit work_stealing_thief(queue_block_ptr bottom)
		: m_bottom(vsm_move(bottom))
	{
	}

	friend class work_stealing_queue<T, Allocator>;
};

} // namespace vsm
