#pragma once

#include <vsm/default_allocator.hpp>
#include <vsm/detail/swiss_table.hpp>
#include <vsm/hash_set.hpp>

namespace vsm {

template<
	typename T,
	typename KeySelector = default_key_selector,
	typename Allocator = default_allocator,
	typename Hasher = default_hasher,
	typename Comparator = std::equal_to<>>
using swiss_set = basic_hash_set<
	detail::swiss_table::table<
		T,
		T,
		hash_table_policies<KeySelector, Hasher, Comparator>,
		Allocator,
		/* Capacity: */ 0>>;

template<
	typename T,
	size_t Capacity, //TODO: Pick a default small capacity.
	typename KeySelector = default_key_selector,
	typename Allocator = default_allocator,
	typename Hasher = default_hasher,
	typename Comparator = std::equal_to<>>
using small_swiss_set = basic_hash_set<
	detail::swiss_table::table<
		T,
		T,
		hash_table_policies<KeySelector, Hasher, Comparator>,
		Allocator,
		Capacity>>;

} // namespace vsm
