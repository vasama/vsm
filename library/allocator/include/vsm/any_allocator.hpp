#pragma once

#include <vsm/allocator.hpp>
#include <vsm/any_ref.hpp>

namespace vsm {
namespace detail {

struct any_memory_resource_allocate
{
	using signature_type = vsm::allocation(size_t) noexcept;

	template<memory_resource Allocator>
	static vsm::allocation invoke(Allocator& allocator, size_t const min_size) noexcept
	{
		return allocator.allocate(min_size);
	}
};

struct any_memory_resource_deallocate
{
	using signature_type = void(vsm::allocation) noexcept;

	template<memory_resource Allocator>
	static void invoke(Allocator& allocator, vsm::allocation const allocation) noexcept
	{
		allocator.deallocate(allocation);
	}
};

struct any_memory_resource_resize
{
	using signature_type = size_t(allocation, size_t) noexcept;

	template<memory_resource Allocator>
	static size_t invoke(
		Allocator&& allocator,
		vsm::allocation const allocation,
		size_t const min_size) noexcept
	{
		return allocators::resize(allocator, allocation, min_size);
	}
};

} // namespace detail

class any_memory_resource_ref
	: any_ref<
		detail::any_memory_resource_allocate,
		detail::any_memory_resource_deallocate,
		detail::any_memory_resource_resize>
{
public:
	template<memory_resource MemoryResource>
	constexpr any_memory_resource_ref(MemoryResource& memory_resource)
		: any_ref(memory_resource)
	{
	}

	template<allocator Allocator>
		requires constructible_from<any_ref, std::in_place_t, Allocator const&>
	any_memory_resource_ref(Allocator const allocator) noexcept
		: any_ref(std::in_place, allocator)
	{
	}

	[[nodiscard]] vsm::allocation allocate(size_t const min_size) const noexcept
	{
		return any_ref::invoke<detail::any_memory_resource_allocate>(min_size);
	}

	void deallocate(vsm::allocation const allocation) const noexcept
	{
		return any_ref::invoke<detail::any_memory_resource_deallocate>(allocation);
	}

	[[nodiscard]] size_t resize(
		vsm::allocation const allocation,
		size_t const min_size) const noexcept
	{
		return any_ref::invoke<detail::any_memory_resource_resize>(allocation, min_size);
	}
};

using any_allocator = any_memory_resource_ref;

} // namespace vsm
