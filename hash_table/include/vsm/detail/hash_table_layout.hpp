#pragma once

#include <vsm/concepts.hpp>
#include <vsm/standard.hpp>
#include <vsm/utility.hpp>

#include <cstddef>

namespace vsm::detail {

template<typename Base, typename P>
struct hash_table_policies_layout : Base
{
	vsm_no_unique_address P policies;

	hash_table_policies_layout() = default;

	explicit hash_table_policies_layout(any_cvref_of<P> auto&& policies)
		: policies(vsm_forward(policies))
	{
	}
};

template<typename Base, typename A>
struct hash_table_allocator_layout : Base
{
	vsm_no_unique_address A allocator;

	hash_table_allocator_layout() = default;

	explicit hash_table_allocator_layout(any_cvref_of<A> auto&& allocator, auto&&... args)
		: allocator(vsm_forward(allocator))
		, Base(vsm_forward(args)...)
	{
	}
};

template<typename Base, typename T, size_t C, size_t ExtraSize = 0>
struct hash_table_storage_layout : Base
{
	using Base::Base;

	std::byte mutable storage alignas(T)[C * sizeof(T) + ExtraSize];
};

template<typename Base, typename T, size_t ExtraSize>
struct hash_table_storage_layout<Base, T, 0, ExtraSize> : Base
{
	using Base::Base;
};

} // namespace vsm::detail
