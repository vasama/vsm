#pragma once

#include <vsm/any_allocator.hpp>
#include <vsm/bit_packing.hpp>

namespace vsm {
namespace detail {

class any_memory_resource_position_type
{
	void* ptr_1;
	void* ptr_2;
};

struct any_memory_resource_get_position
{
	using signature_type = any_memory_resource_position_type() const noexcept;

	template<monotonic_memory_resource Allocator>
	static any_memory_resource_position_type invoke(Allocator const& allocator) noexcept
	{
		return vsm::bit_pack<any_memory_resource_position_type>(allocator.get_position());
	}
};

struct any_memory_resource_reset_position
{
	using signature_type = void(any_memory_resource_position_type) noexcept;

	template<monotonic_memory_resource Allocator>
	static void invoke(Allocator& allocator, any_memory_resource_position_type const pos) noexcept
	{
		allocator.reset_position(vsm::bit_unpack<typename Allocator::position_type>(pos));
	}
};

} // namespace detail

class any_monotonic_memory_resource_ref
	: public any_ref<
		detail::any_memory_resource_allocate,
		detail::any_memory_resource_deallocate,
		detail::any_memory_resource_resize,
		detail::any_memory_resource_get_position,
		detail::any_memory_resource_reset_position>
{
public:
	using position_type = detail::any_memory_resource_position_type;

	template<no_cvref_of<any_monotonic_memory_resource_ref> MemoryResource>
		requires monotonic_memory_resource<MemoryResource>
	constexpr any_monotonic_memory_resource_ref(MemoryResource& memory_resource)
		: any_ref(memory_resource)
	{
	}

	template<no_cvref_of<any_monotonic_memory_resource_ref> AnyMemoryResourceRef>
		requires constructible_from<any_ref, typename AnyMemoryResourceRef::any_ref>
	constexpr any_monotonic_memory_resource_ref(AnyMemoryResourceRef const& ref)
		: any_ref(ref)
	{
	}

	template<no_cvref_of<any_monotonic_memory_resource_ref> Allocator>
		requires
			allocator<Allocator> &&
			constructible_from<any_ref, std::in_place_t, Allocator const&>
	any_monotonic_memory_resource_ref(Allocator const allocator) noexcept
		: any_ref(std::in_place, allocator)
	{
	}

	[[nodiscard]] vsm::allocation allocate(
		size_t const min_size,
		size_t const max_size) const noexcept
	{
		return any_ref::invoke<detail::any_memory_resource_allocate>(min_size, max_size);
	}

	void deallocate(vsm::allocation const allocation) const noexcept
	{
		return any_ref::invoke<detail::any_memory_resource_deallocate>(allocation);
	}

	[[nodiscard]] size_t resize(
		vsm::allocation const allocation,
		size_t const min_size,
		size_t const max_size) const noexcept
	{
		return any_ref::invoke<detail::any_memory_resource_resize>(allocation, min_size, max_size);
	}

	[[nodiscard]] position_type get_position() const noexcept
	{
		return any_ref::invoke<detail::any_memory_resource_get_position>();
	}

	void reset_position(position_type const pos) const noexcept
	{
		any_ref::invoke<detail::any_memory_resource_reset_position>(pos);
	}

	[[nodiscard]] operator any_allocator() const noexcept
	{
		return any_allocator(*this);
	}
};

using any_monotonic_allocator = any_monotonic_memory_resource_ref;

} // namespace vsm
