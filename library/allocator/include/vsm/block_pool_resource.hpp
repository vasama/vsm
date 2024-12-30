#pragma once

#include <vsm/allocator.hpp>
#include <vsm/any_allocator.hpp>
#include <vsm/assert.h>
#include <vsm/numeric.hpp>
#include <vsm/standard.hpp>
#include <vsm/utility.hpp>

namespace vsm {

class block_pool_size_policy
{
	uint32_t m_block_size;
	uint32_t m_chunk_mult;

public:
	block_pool_size_policy(size_t const block_size) noexcept
		: m_block_size(block_size)
		, m_chunk_mult(1)
	{
	}

	block_pool_size_policy(size_t const block_size, size_t const chunk_size)
		: m_block_size(vsm::truncating(block_size))
		, m_chunk_mult(vsm::truncating(chunk_size / block_size))
	{
		vsm_assert(chunk_size % block_size == 0);
	}

	[[nodiscard]] size_t block_size() const noexcept
	{
		return m_block_size;
	}

	[[nodiscard]] size_t chunk_size() const noexcept
	{
		return m_block_size * m_chunk_mult;
	}
};

template<memory_resource MemoryResource, typename SizePolicy = block_pool_size_policy>
class basic_block_pool_resource
{
	struct free_block
	{
		free_block* next;

		size_t size;
		void* free[];
	};

	static constexpr size_t min_block_size = offsetof(free_block, free);

	vsm_no_unique_address SizePolicy m_size_policy;
	vsm_no_unique_address MemoryResource m_backing_resource;

	free_block* m_free_head;

public:
	basic_block_pool_resource()
		requires
			std::is_default_constructible_v<SizePolicy> &&
			std::is_default_constructible_v<MemoryResource>
		= default;

	explicit basic_block_pool_resource(SizePolicy const& size_policy) noexcept
		requires std::is_default_constructible_v<MemoryResource>
		: basic_block_pool_resource(size_policy, std::in_place)
	{
	}

	template<typename... Args>
		requires
			std::is_default_constructible_v<SizePolicy> &&
			std::constructible_from<MemoryResource, Args...>
	explicit basic_block_pool_resource(
		std::in_place_t,
		Args&&... args)
		noexcept(std::is_nothrow_constructible_v<MemoryResource, Args...>)
		: m_backing_resource(vsm_forward(args)...)
	{
	}

	template<typename... Args>
		requires std::constructible_from<MemoryResource, Args...>
	explicit basic_block_pool_resource(
		SizePolicy const& size_policy,
		std::in_place_t,
		Args&&... args)
		noexcept(std::is_nothrow_constructible_v<MemoryResource, Args...>)
		: m_size_policy(size_policy)
		, m_backing_resource(vsm_forward(args)...)
	{
	}


	basic_block_pool_resource(basic_block_pool_resource const&) = delete;
	basic_block_pool_resource& operator=(basic_block_pool_resource const&) = delete;


	[[nodiscard]] MemoryResource const& backing_resource() const
	{
		return m_backing_resource;
	}

	[[nodiscard]] size_t block_size() const noexcept
	{
		return m_size_policy.block_size();
	}


	[[nodiscard]] allocation allocate(size_t const min_size, size_t const max_size) noexcept
	{
		size_t const block_size = m_size_policy.block_size();

		if (min_size > block_size)
		{
			return allocation(nullptr);
		}

		void* const block = acquire_block();

		if (block == nullptr)
		{
			return allocation(nullptr);
		}

		return allocation(block, block_size);
	}

	void deallocate(vsm::allocation const allocation) noexcept
	{
		vsm_assert(allocation.size <= m_size_policy.block_size());
		release_block(allocation.storage);
	}

private:
	[[nodiscard]] void* acquire_block()
	{
		if (free_block* const head = m_free_head)
		{
			if (head->size == 0)
			{
				m_free_head = head->next;
				return head;
			}

			return head->free[--head->size];
		}

		return acquire_new_blocks();
	}

	void release_block(void* const block)
	{
		free_block* const head = m_free_head;

		if (head == nullptr || head->size == get_max_nested())
		{
			m_free_head = new (block) free_block();
			m_free_head->next = head;
		}
		else
		{
			head->free[head->size++] = block;
		}
	}

	[[nodiscard]] void* acquire_new_blocks()
	{
		size_t const block_size = m_size_policy.block_size();
		size_t const chunk_size = m_size_policy.chunk_size();

		auto const allocation = vsm::allocate(m_backing_resource, chunk_size);

		if (allocation.storage == nullptr)
		{
			return nullptr;
		}

		auto const new_blocks = reinterpret_cast<std::byte*>(allocation.storage);
		size_t const new_block_count = allocation.size / block_size;

		for (size_t i = 1; i < new_block_count; ++i)
		{
			release_block(new_blocks + i * block_size);
		}

		return new_blocks;
	}

	[[nodiscard]] size_t get_max_nested() const
	{
		return (m_size_policy.block_size() - min_block_size) / sizeof(void*);
	}
};

using block_pool_resource = basic_block_pool_resource<any_allocator>;

} // namespace vsm
