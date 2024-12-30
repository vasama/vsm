#pragma once

#include <vsm/concepts.hpp>
#include <vsm/key_selector.hpp>
#include <vsm/standard.hpp>
#include <vsm/utility.hpp>

#include <cstddef>

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
inline vsm_always_inline decltype(auto) get_lookup_key(
	KeySelector const& key_selector,
	K const& key)
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

} // namespace detail
} // namespace vsm
