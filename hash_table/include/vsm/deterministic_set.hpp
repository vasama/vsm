#pragma once

#include <vsm/detail/deterministic_table.hpp>
#include <vsm/hash_set.hpp>

namespace vsm {

template<
	typename T,
	typename KeySelector = default_key_selector,
	typename Allocator = default_allocator,
	typename Hasher = default_hasher,
	typename Comparator = std::equal_to<>>
using deterministic_set = basic_hash_set<
	detail::deterministic_table::table<
		T,
		T,
		uint_least32_t,
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
using small_deterministic_map = basic_hash_set<
	detail::deterministic_table::table<
		T,
		T,
		uint_least32_t,
		hash_table_policies<KeySelector, Hasher, Comparator>,
		Allocator,
		Capacity>>;

} // namespace vsm
