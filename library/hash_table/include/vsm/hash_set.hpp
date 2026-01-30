#pragma once

#include <vsm/concepts.hpp>
#include <vsm/detail/hash_table.hpp>
#include <vsm/key_value_pair.hpp>
#include <vsm/standard/stdexcept.hpp>

#include <memory>
#include <new>

namespace vsm {

template<non_cvref HashTableBase>
class new_basic_hash_set_base : public HashTableBase
{
public:
	using HashTableBase::HashTableBase;


	template<detail::hash_table_key<HashTableBase> K>
	[[nodiscard]] typename HashTableBase::value_type* find_ptr(K const& key)
	{
		auto const it = HashTableBase::find(key);
		return it != HashTableBase::end() ? &*it : nullptr;
	}

	template<detail::hash_table_key<HashTableBase> K>
	[[nodiscard]] typename HashTableBase::value_type const* find_ptr(K const& key) const
	{
		auto const it = HashTableBase::find(key);
		return it != HashTableBase::end() ? &*it : nullptr;
	}

	template<detail::hash_table_key<HashTableBase> K>
	[[nodiscard]] size_t count(K const& key) const
	{
		return HashTableBase::find(key) != HashTableBase::end();
	}

	template<detail::hash_table_key<HashTableBase> K>
	[[nodiscard]] bool contains(K const& key) const
	{
		return HashTableBase::find(key) != HashTableBase::end();
	}


	template<detail::hash_table_key<HashTableBase> K>
	typename HashTableBase::insert_result insert(K&& key)
	{
		auto const r = HashTableBase::insert_uninitialized(vsm_as_const(key));

		if (r.inserted)
		{
			::new (std::to_address(r.iterator)) typename HashTableBase::value_type(
				vsm_forward(key));
		}

		return r;
	}

	template<detail::hash_table_key<HashTableBase> K>
	typename HashTableBase::insert_result insert_or_assign(K&& key)
	{
		auto const r = HashTableBase::insert_uninitialized(vsm_as_const(key));

		if (r.inserted)
		{
			::new (std::to_address(r.iterator)) typename HashTableBase::value_type(
				vsm_forward(key));
		}
		else
		{
			*r.iterator = vsm_forward(key);
		}

		return r;
	}

	template<typename... Args>
		requires std::constructible_from<value_type, Args...>
	typename HashTableBase::insert_result emplace(Args&&... args)
	{
		typename HashTableBase::value_type new_element(vsm_forward(args)...);

		auto const r = HashTableBase::insert_uninitialized(vsm_as_const(new_element));

		if (r.inserted)
		{
			::new (std::to_address(r.iterator)) typename HashTableBase::value_type(
				vsm_move(new_element));
		}

		return r;
	}

	template<typename... Args>
	typename HashTableBase::insert_result emplace_or_assign(Args&&... args)
	{
		typename HashTableBase::value_type new_element(vsm_forward(args)...);

		auto const r = HashTableBase::insert_uninitialized(vsm_as_const(new_element));

		if (r.inserted)
		{
			::new (std::to_address(r.iterator)) typename HashTableBase::value_type(
				vsm_move(new_element));
		}
		else
		{
			*r.iterator = vsm_move(new_element);
		}

		return r;
	}


	[[nodiscard]] auto vsm_always_inline cbegin() const
	{
		return HashTableBase::begin();
	}

	[[nodiscard]] auto vsm_always_inline cend() const
	{
		return HashTableBase::end();
	}
};

} // namespace vsm
