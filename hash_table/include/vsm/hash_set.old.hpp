#pragma once

#include <vsm/default_hash.hpp>
#include <vsm/detail/hash_table.hpp>
#include <vsm/key_selector.hpp>

#include <vsm/detail/swiss_table.hpp>

#include <memory>
#include <new>

namespace vsm {

template<typename T, typename Policies, typename Allocator, size_t Capacity>
class basic_hash_set : public detail::swiss_table::hash_table<T, T, Policies, Allocator, Capacity>
{
	using b = detail::swiss_table::hash_table<T, T, Policies, Allocator, Capacity>;

public:
	using element_type = T;

	using insert_result = vsm::insert_result<element_type>;


	using b::b;


	using b::find;

	template<typename K>
	[[nodiscard]] bool contains(K const& key) const
	{
		return b::find(key) != nullptr;
	}


	template<typename K>
	insert_result insert(K&& key)
	{
		insert_result r = b::insert(vsm_as_const(key));

		if (r.inserted)
		{
			::new (r.element) element_type(vsm_forward(key));
		}

		return r;
	}

	template<typename K>
	insert_result insert_or_assign(K&& key)
	{
		insert_result r = b::insert(vsm_as_const(key));

		if (r.inserted)
		{
			::new (r.element) element_type(vsm_forward(key));
		}
		else
		{
			*r.element = vsm_forward(key);
		}

		return r;
	}

	template<typename... Args>
	insert_result emplace(Args&&... args)
	{
		element_type new_element(vsm_forward(args)...);
	
		insert_result r = b::insert(vsm_as_const(new_element));

		if (r.inserted)
		{
			::new (r.element) element_type(vsm_move(new_element));
		}

		return r;
	}

	template<typename... Args>
	insert_result emplace_or_assign(Args&&... args)
	{
		element_type new_element(vsm_forward(args)...);
	
		insert_result r = b::insert(vsm_as_const(new_element));

		if (r.inserted)
		{
			::new (r.element) element_type(vsm_move(new_element));
		}
		else
		{
			*r.element = vsm_move(new_element);
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
	typename T,
	typename KeySelector = default_key_selector,
	typename Hasher = basic_hasher<default_hash>,
	typename Comparator = std::equal_to<>,
	typename Allocator = default_allocator>
using hash_set = basic_hash_set<T, hash_table_policies<KeySelector, Hasher, Comparator>, Allocator, 0>;

template<
	typename T, size_t Capacity,
	typename KeySelector = default_key_selector,
	typename Hasher = basic_hasher<default_hash>,
	typename Comparator = std::equal_to<>,
	typename Allocator = default_allocator>
using small_hash_set = basic_hash_set<T, hash_table_policies<KeySelector, Hasher, Comparator>, Allocator, Capacity>;

} // namespace vsm
