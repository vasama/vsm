#pragma once

#include <vsm/allocator.hpp>
#include <vsm/detail/deterministic_table.hpp>
#include <vsm/hash_map.hpp>

namespace vsm {

template<
	typename Key,
	typename Value,
	typename KeySelector = default_key_selector,
	typename Allocator = default_allocator,
	typename Hasher = default_hasher,
	typename Comparator = std::equal_to<>>
using deterministic_map = basic_hash_map<
	detail::deterministic_table::table<
		key_value_pair<Key, Value>,
		Key,
		uint_least32_t,
		hash_table_policies<KeySelector, Hasher, Comparator>,
		Allocator,
		0>>;

template<
	typename Key,
	typename Value,
	size_t Capacity, //TODO: Pick a default small capacity.
	typename KeySelector = default_key_selector,
	typename Allocator = default_allocator,
	typename Hasher = default_hasher,
	typename Comparator = std::equal_to<>>
using small_deterministic_map = basic_hash_map<
	detail::deterministic_table::table<
		key_value_pair<Key, Value>,
		Key,
		uint_least32_t,
		hash_table_policies<KeySelector, Hasher, Comparator>,
		Allocator,
		Capacity>>;

} // namespace vsm
