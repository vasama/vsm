#pragma once

#include <vsm/default_hash.hpp>
#include <vsm/detail/swiss_table.hpp>
#include <vsm/hash_set.hpp>
#include <vsm/key_selector.hpp>

namespace vsm {

template<
	typename T,
	typename KeySelector = default_key_selector,
	typename Allocator = default_allocator,
	typename Hasher = default_hasher,
	typename Comparator = std::equal_to<>>
using swiss_set_base = new_basic_hash_set_base<
	detail::_swiss_table_base_impl<
		T,
		T,
		hash_table_policies<KeySelector, Hasher, Comparator>,
		Allocator>>;

template<
	typename T,
	typename KeySelector = default_key_selector,
	typename Allocator = default_allocator,
	typename Hasher = default_hasher,
	typename Comparator = std::equal_to<>>
using swiss_set = detail::_swiss_table_impl<
	swiss_set_base<T, KeySelector, Allocator, Hasher, Comparator>,
	/* Capacity: */ 0>;

template<
	typename T,
	size_t Capacity, // TODO: Pick a default small capacity.
	typename KeySelector = default_key_selector,
	typename Allocator = default_allocator,
	typename Hasher = default_hasher,
	typename Comparator = std::equal_to<>>
using small_swiss_set = detail::_swiss_table_impl<
	swiss_set_base<T, KeySelector, Allocator, Hasher, Comparator>,
	Capacity>;

} // namespace vsm
