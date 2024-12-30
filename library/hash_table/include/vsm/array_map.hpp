#pragma once

#include <vsm/default_hash.hpp>
#include <vsm/detail/array_table.hpp>
#include <vsm/hash_map.hpp>
#include <vsm/key_selector.hpp>

namespace vsm {

template<
	typename Key,
	typename Value,
	typename KeySelector = default_key_selector,
	typename Allocator = default_allocator,
	typename Hasher = default_hasher,
	typename Comparator = std::equal_to<>>
using array_map_base = new_basic_hash_map_base<
	detail::_array_table_base_impl<
		key_value_pair<Key, Value>,
		Key,
		uint_least32_t,
		hash_table_policies<KeySelector, Hasher, Comparator>,
		Allocator>>;

template<
	typename Key,
	typename Value,
	typename KeySelector = default_key_selector,
	typename Allocator = default_allocator,
	typename Hasher = default_hasher,
	typename Comparator = std::equal_to<>>
using array_map = detail::_array_table_impl<
	array_map_base<Key, Value, KeySelector, Allocator, Hasher, Comparator>,
	/* Capacity: */ 0>;

template<
	typename Key,
	typename Value,
	size_t Capacity, // TODO: Pick a default small capacity.
	typename KeySelector = default_key_selector,
	typename Allocator = default_allocator,
	typename Hasher = default_hasher,
	typename Comparator = std::equal_to<>>
using small_array_map = detail::_array_table_impl<
	array_map_base<Key, Value, KeySelector, Allocator, Hasher, Comparator>,
	Capacity>;

} // namespace vsm
