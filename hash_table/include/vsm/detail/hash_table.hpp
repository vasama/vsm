#pragma once

#include <vsm/standard.hpp>

namespace vsm {

template<typename KeySelector, typename Hasher, typename Comparator>
struct hash_table_policies
{
	using key_selector_type = KeySelector;
	using hasher_type = Hasher;
	using comparator_type = Comparator;

	vsm_no_unique_address KeySelector key_selector;
	vsm_no_unique_address Hasher hasher;
	vsm_no_unique_address Comparator comparator;
};

namespace detail {

template<typename Key, typename KeySelector, typename K>
vsm_always_inline decltype(auto) get_lookup_key(KeySelector const& key_selector, K const& key)
{
	if constexpr (std::is_same_v<K, Key> || requires { key_selector(key); })
	{
		return key_selector(key);
	}
	else
	{
		return key;
	}
}

template<typename Key, typename Table>
concept _hash_table_key = requires (Key const& key, typename Table::policies_type const& p)
{
	p.hasher(normalize_key(key));
	p.comparator(normalize_key(key), std::declval<typename Table::key_type const&>());
};

template<typename Key, typename Table>
concept hash_table_key = requires (Key const& key, typename Table::policies_type const& p)
{
	{ get_lookup_key<typename Table::key_type>(p.key_selector, key) } -> _hash_table_key<Table>;
};

} // namespace detail
} // namespace vsm
