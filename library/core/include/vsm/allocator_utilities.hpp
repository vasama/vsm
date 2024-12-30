#pragma once

#include <vsm/allocator.hpp>
#include <vsm/exceptions.hpp>

#include <ranges>
#include <span>

namespace vsm {

template<non_decaying T, memory_resource MemoryResource>
[[nodiscard]] std::span<T> new_array_via(MemoryResource&& memory_resource, size_t const size)
{
	vsm::allocation const storage = vsm::allocate_or_throw(
		memory_resource,
		size * sizeof(T));

	T* const data = static_cast<T*>(storage.storage);

	vsm_except_try
	{
		std::uninitialized_default_construct_n(data, size);
	}
	vsm_except_catch (...)
	{
		memory_resource.deallocate(storage);
	}

	return std::span<T>(data, size);
}

template<memory_resource MemoryResource, std::ranges::sized_range Range>
[[nodiscard]] std::span<std::ranges::range_value_t<Range>> new_array_via(
	MemoryResource&& memory_resource,
	Range&& range)
{
	using value_type = std::ranges::range_value_t<Range>;

	size_t const size = std::ranges::size(range);

	vsm::allocation const storage = vsm::allocate_or_throw(
		memory_resource,
		size * sizeof(value_type));

	value_type* const data = static_cast<value_type*>(storage.storage);

	vsm_except_try
	{
		std::uninitialized_copy_n(std::ranges::begin(range), size, data);
	}
	vsm_except_catch (...)
	{
		memory_resource.deallocate(storage);
	}

	return std::span<value_type>(data, size);
}

template<memory_resource MemoryResource, typename Char>
[[nodiscard]] std::basic_string_view<Char> new_string_via(
	MemoryResource&& memory_resource,
	std::basic_string_view<Char> const string)
{
	return std::basic_string_view<Char>(new_array_via(vsm_forward(memory_resource), string));
}

} // namespace vsm
