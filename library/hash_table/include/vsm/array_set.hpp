#pragma once

#include <vsm/default_hash.hpp>
#include <vsm/detail/array_table.hpp>
#include <vsm/hash_set.hpp>
#include <vsm/key_selector.hpp>

namespace vsm {

template<
	typename T,
	typename KeySelector = default_key_selector,
	typename Allocator = default_allocator,
	typename Hasher = default_hasher,
	typename Comparator = std::equal_to<>>
using array_set_base = new_basic_hash_set_base<
	detail::_array_table_base_impl<
		T,
		T,
		uint_least32_t,
		hash_table_policies<KeySelector, Hasher, Comparator>,
		Allocator>>;

template<
	typename T,
	typename KeySelector = default_key_selector,
	typename Allocator = default_allocator,
	typename Hasher = default_hasher,
	typename Comparator = std::equal_to<>>
using array_set = detail::_array_table_impl<
	array_set_base<T, KeySelector, Allocator, Hasher, Comparator>,
	/* Capacity: */ 0>;

template<
	typename T,
	size_t Capacity, // TODO: Pick a default small capacity.
	typename KeySelector = default_key_selector,
	typename Allocator = default_allocator,
	typename Hasher = default_hasher,
	typename Comparator = std::equal_to<>>
using small_array_set = detail::_array_table_impl<
	array_set_base<T, KeySelector, Allocator, Hasher, Comparator>,
	Capacity>;

} // namespace vsm
