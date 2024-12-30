#pragma once

#include <vsm/concepts.hpp>
#include <vsm/detail/hash_table.hpp>
#include <vsm/key_value_pair.hpp>
#include <vsm/standard/stdexcept.hpp>

#include <memory>
#include <new>

namespace vsm {

template<non_cvref HashTableBase>
class new_basic_hash_map_base : public HashTableBase
{
public:
	using mapped_type           = typename HashTableBase::value_type::value_type;

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
	[[nodiscard]] mapped_type& at(K const& key)
	{
		auto const it = HashTableBase::find(key);
		if (it == HashTableBase::end())
		{
			vsm_except_throw_or_terminate(std::out_of_range("hash map key not found"));
		}
		return it->value;
	}

	template<detail::hash_table_key<HashTableBase> K>
	[[nodiscard]] mapped_type const& at(K const& key) const
	{
		auto const it = HashTableBase::find(key);
		if (it == HashTableBase::end())
		{
			vsm_except_throw_or_terminate(std::out_of_range("hash map key not found"));
		}
		return it->value;
	}

	template<detail::hash_table_key<HashTableBase> K>
	[[nodiscard]] mapped_type* at_ptr(K const& key)
	{
		auto const it = HashTableBase::find(key);
		return it != HashTableBase::end() ? &it->value : nullptr;
	}

	template<detail::hash_table_key<HashTableBase> K>
	[[nodiscard]] mapped_type const* at_ptr(K const& key) const
	{
		auto const it = HashTableBase::find(key);
		return it != HashTableBase::end() ? &it->value : nullptr;
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

	template<detail::hash_table_key<HashTableBase> K, std::convertible_to<mapped_type> V>
	typename HashTableBase::insert_result insert(K&& key, V&& value)
	{
		auto const r = HashTableBase::insert_uninitialized(vsm_as_const(key));

		if (r.inserted)
		{
			::new (std::to_address(r.iterator)) typename HashTableBase::value_type(
				vsm_forward(key),
				vsm_forward(value));
		}

		return r;
	}

	template<detail::hash_table_key<HashTableBase> K, typename... Args>
		requires std::constructible_from<mapped_type, Args...>
	typename HashTableBase::insert_result try_emplace(K&& key, Args&&... args)
	{
		auto const r = HashTableBase::insert_uninitialized(vsm_as_const(key));

		if (r.inserted)
		{
			::new (std::to_address(r.iterator)) typename HashTableBase::value_type(
				vsm_forward(key),
				mapped_type(vsm_forward(args)...));
		}

		return r;
	}

	template<detail::hash_table_key<HashTableBase> K, std::convertible_to<mapped_type> V>
	typename HashTableBase::insert_result insert_or_assign(K&& key, V&& value)
	{
		auto const r = HashTableBase::insert_uninitialized(vsm_as_const(key));

		if (r.inserted)
		{
			::new (std::to_address(r.iterator)) typename HashTableBase::value_type(
				vsm_forward(key),
				vsm_forward(value));
		}
		else
		{
			r.iterator->value = vsm_forward(value);
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
