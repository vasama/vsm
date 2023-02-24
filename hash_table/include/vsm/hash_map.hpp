#pragma once

namespace vsm {
namespace detail::hash_table_ {

template<typename Key, typename Value, typename Allocator, size_t LocalCapacity>
class hash_map
{

};

} // namespace detail::hash_table_

template<typename Key, typename Value, typename Allocator>
using hash_map = detail::hash_table_::hash_map<Key, Value, Allocator, 0>;

template<typename Key, typename Value, size_t LocalCapacity, typename Allocator>
using small_hash_map = detail::hash_table_::hash_map<Key, Value, Allocator, LocalCapacity>;

} // namespace vsm
