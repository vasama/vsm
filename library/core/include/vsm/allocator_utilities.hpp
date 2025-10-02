#pragma once

#include <vsm/allocator.hpp>
#include <vsm/exceptions.hpp>

#include <ranges>
#include <span>

namespace vsm {

template<non_decaying T, memory_resource Allocator>
[[nodiscard]] std::span<T> new_array_via(Allocator&& allocator, size_t const size)
{
	vsm::allocation const storage = vsm::allocate_or_throw(
		allocator,
		size * sizeof(T));

	T* const data = static_cast<T*>(storage.storage);

	vsm_except_try
	{
		std::uninitialized_default_construct_n(data, size);
	}
	vsm_except_catch (...)
	{
		allocator.deallocate(storage);
	}

	return std::span<T>(data, size);
}

template<memory_resource Allocator, std::ranges::sized_range Range>
std::span<std::ranges::range_value_t<Range>> new_array_via(
	Allocator&& allocator,
	Range&& range)
{
	using value_type = std::ranges::range_value_t<Range>;

	size_t const size = std::ranges::size(range);

	vsm::allocation const storage = vsm::allocate_or_throw(
		allocator,
		size * sizeof(value_type));

	value_type* const data = static_cast<value_type*>(storage.storage);

	vsm_except_try
	{
		std::uninitialized_copy_n(std::ranges::begin(range), size, data);
	}
	vsm_except_catch (...)
	{
		allocator.deallocate(storage);
	}

	return std::span<value_type>(data, size);
}

template<memory_resource MemoryResource>
std::string_view new_string_via(MemoryResource&& memory_resource, std::string_view const string)
{
	return std::string_view(new_array_via(vsm_forward(memory_resource), string));
}

} // namespace vsm
