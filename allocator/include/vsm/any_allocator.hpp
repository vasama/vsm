#pragma once

#include <vsm/allocator.hpp>
#include <vsm/any_ref.hpp>

namespace vsm {
namespace detail {

struct any_allocator_allocate
{
	using signature_type = allocation(size_t);

	template<memory_resource Allocator>
	static allocation invoke(Allocator&& allocator, size_t const min_size)
	{
		return allocator.allocate(min_size);
	}
};

struct any_allocator_deallocate
{
	using signature_type = void(allocation);

	template<memory_resource Allocator>
	static void invoke(Allocator&& allocator, vsm::allocation const allocation)
	{
		allocator.deallocate(allocation);
	}
};

struct any_allocator_resize
{
	using signature_type = size_t(allocation, size_t);

	template<memory_resource Allocator>
	static size_t invoke(
		Allocator&& allocator,
		vsm::allocation const allocation,
		size_t const min_size)
	{
		return allocators::resize(allocator, allocation, min_size);
	}
};

} // namespace detail

class any_allocator
	: any_ref<
		detail::any_allocator_allocate,
		detail::any_allocator_deallocate,
		detail::any_allocator_resize>
{
public:
	template<memory_resource MemoryResource>
	constexpr any_allocator(MemoryResource& memory_resource)
		: any_ref(memory_resource)
	{
	}

	template<allocator Allocator>
		requires std::is_empty_v<Allocator>
	constexpr any_allocator(Allocator const allocator)
		: any_ref(std::in_place, allocator)
	{
	}

	[[nodiscard]] constexpr vsm::allocation allocate(size_t const min_size) const
	{
		return any_ref::invoke<detail::any_allocator_allocate>(min_size);
	}

	constexpr void deallocate(vsm::allocation const allocation) const
	{
		return any_ref::invoke<detail::any_allocator_deallocate>(allocation);
	}

	[[nodiscard]] constexpr size_t resize(vsm::allocation const allocation, size_t const min_size) const
	{
		return any_ref::invoke<detail::any_allocator_resize>(allocation, min_size);
	}
};

} // namespace vsm
