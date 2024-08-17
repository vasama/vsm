#pragma once

#include <vsm/default_hash.hpp>
#include <vsm/detail/hash_table.hpp>
#include <vsm/key_value_pair.hpp>
#include <vsm/key_selector.hpp>

#include <vsm/detail/swiss_table.hpp>

#include <memory>
#include <new>

namespace vsm {

template<typename Key, typename Value, typename Policies, typename Allocator, size_t Capacity>
class basic_hash_map : public detail::swiss_table::hash_table<key_value_pair<Key, Value>, Key, Policies, Allocator, Capacity>
{
	using b = detail::swiss_table::hash_table<key_value_pair<Key, Value>, Key, Policies, Allocator, Capacity>;

public:
	using element_type = key_value_pair<Key, Value>;

	using key_type = Key;
	using value_type = Value;

	using insert_result = vsm::insert_result<element_type>;


	using b::b;


	using b::find;

	template<typename K>
	[[nodiscard]] bool contains_key(K const& key) const
	{
		return b::find(key) != nullptr;
	}

	template<typename K>
	[[nodiscard]] Value* find_value(K const& key)
	{
		element_type* const element = b::find(key);
		return element != nullptr ? &element->value : nullptr;
	}

	template<typename K>
	[[nodiscard]] Value const* find_value(K const& key) const
	{
		element_type const* const element = b::find(key);
		return element != nullptr ? &element->value : nullptr;
	}

	template<typename K>
	[[nodiscard]] Value& operator[](K const& key)
	{
		element_type* const element = b::find(key);
		vsm_assert(element != nullptr);
		return element->value;
	}

	template<typename K>
	[[nodiscard]] Value const& operator[](K const& key) const
	{
		element_type const* const element = b::find(key);
		vsm_assert(element != nullptr);
		return element->value;
	}


	template<typename K>
	insert_result insert(K&& key)
	{
		insert_result r = b::insert(vsm_as_const(key));

		if (r.inserted)
		{
			::new (r.element) element_type{ vsm_forward(key) };
		}

		return r;
	}

	template<typename K, typename V>
	insert_result insert(K&& key, V&& value)
	{
		insert_result r = b::insert(vsm_as_const(key));

		if (r.inserted)
		{
			::new (r.element) element_type{ vsm_forward(key), vsm_forward(value) };
		}

		return r;
	}

	template<typename K, typename... Args>
	insert_result emplace(K&& key, Args&&... args)
	{
		insert_result r = b::insert(vsm_as_const(key));

		if (r.inserted)
		{
			::new (r.element) element_type{ vsm_forward(key), { vsm_forward(args)... } };
		}

		return r;
	}

	template<typename K, typename V>
	insert_result insert_or_assign(K&& key, V&& value)
	{
		insert_result r = b::insert(vsm_as_const(key));

		if (r.inserted)
		{
			::new (r.element) element_type{ vsm_forward(key), vsm_forward(value) };
		}
		else
		{
			*r.element.value = vsm_forward(value);
		}

		return r;
	}

	template<typename K>
	bool remove(K const& key)
	{
		if (element_type* const element = b::remove(key))
		{
			element->~element_type();
			return true;
		}
		return false;
	}
};

template<
	typename Key,
	typename Value,
	typename KeySelector = default_key_selector,
	typename Hasher = basic_hasher<default_hash>,
	typename Comparator = std::equal_to<>,
	typename Allocator = default_allocator>
using hash_map = basic_hash_map<
	Key,
	Value,
	hash_table_policies<KeySelector, Hasher, Comparator>,
	Allocator,
	0>;

template<
	typename Key,
	typename Value,
	size_t Capacity,
	typename KeySelector = default_key_selector,
	typename Hasher = basic_hasher<default_hash>,
	typename Comparator = std::equal_to<>,
	typename Allocator = default_allocator>
using small_hash_map = basic_hash_map<
	Key,
	Value,
	hash_table_policies<KeySelector, Hasher, Comparator>,
	Allocator,
	Capacity>;

} // namespace vsm
