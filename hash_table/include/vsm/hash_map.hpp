#pragma once

#include <vsm/concepts.hpp>
#include <vsm/default_hash.hpp>
#include <vsm/detail/hash_table.hpp>
#include <vsm/key_value_pair.hpp>
#include <vsm/standard/stdexcept.hpp>

#include <memory>
#include <new>

namespace vsm {

template<non_cvref Table>
class basic_hash_map : public Table
{
public:
	using element_type          = typename Table::element_type;
	using key_type              = typename element_type::key_type;
	using mapped_type           = typename element_type::value_type;

private:
	using ordered_iterator_or_void = select_t<Table::is_ordered, typename Table::iterator, void>;

public:
	using Table::Table;


	template<detail::hash_table_key<Table> K>
	[[nodiscard]] element_type* find_ptr(K const& key)
	{
		auto const it = Table::find(key);
		return it != Table::end() ? &*it : nullptr;
	}

	template<detail::hash_table_key<Table> K>
	[[nodiscard]] element_type const* find_ptr(K const& key) const
	{
		auto const it = Table::find(key);
		return it != Table::end() ? &*it : nullptr;
	}

	template<detail::hash_table_key<Table> K>
	[[nodiscard]] mapped_type& at(K const& key)
	{
		auto const it = Table::find(key);
		if (it == Table::end())
		{
			vsm_except_throw_or_terminate(std::out_of_range("hash map key not found"));
		}
		return it->value;
	}

	template<detail::hash_table_key<Table> K>
	[[nodiscard]] mapped_type const& at(K const& key) const
	{
		auto const it = Table::find(key);
		if (it == Table::end())
		{
			vsm_except_throw_or_terminate(std::out_of_range("hash map key not found"));
		}
		return it->value;
	}

	template<detail::hash_table_key<Table> K>
	[[nodiscard]] mapped_type* at_ptr(K const& key)
	{
		auto const it = Table::find(key);
		return it != Table::end() ? &it->value : nullptr;
	}

	template<detail::hash_table_key<Table> K>
	[[nodiscard]] mapped_type const* at_ptr(K const& key) const
	{
		auto const it = Table::find(key);
		return it != Table::end() ? &it->value : nullptr;
	}

	template<detail::hash_table_key<Table> K>
	[[nodiscard]] size_t count(K const& key) const
	{
		return Table::find(key) != Table::end();
	}

	template<detail::hash_table_key<Table> K>
	[[nodiscard]] bool contains(K const& key) const
	{
		return Table::find(key) != Table::end();
	}


	template<detail::hash_table_key<Table> K>
	typename Table::insert_result insert(K&& key)
	{
		auto const r = Table::insert(vsm_as_const(key));

		if (r.inserted)
		{
#ifndef __INTELLISENSE__
// https://developercommunity.visualstudio.com/t/EDG-rejects-parenthesized-aggregate-init/10732572

			::new (std::to_address(r.iterator)) element_type(
				vsm_forward(key));
#endif
		}

		return r;
	}

	template<detail::hash_table_key<Table> K, std::convertible_to<mapped_type> V>
	typename Table::insert_result insert(K&& key, V&& value)
	{
		auto const r = Table::insert(vsm_as_const(key));

		if (r.inserted)
		{
#ifndef __INTELLISENSE__
			::new (std::to_address(r.iterator)) element_type(
				vsm_forward(key),
				vsm_forward(value));
#endif
		}

		return r;
	}

	template<detail::hash_table_key<Table> K, typename... Args>
		requires std::constructible_from<mapped_type, Args...>
	typename Table::insert_result try_emplace(K&& key, Args&&... args)
	{
		auto const r = Table::insert(vsm_as_const(key));

		if (r.inserted)
		{
#ifndef __INTELLISENSE__
			::new (std::to_address(r.iterator)) element_type(
				vsm_forward(key),
				mapped_type(vsm_forward(args)...));
#endif
		}

		return r;
	}

	template<detail::hash_table_key<Table> K, std::convertible_to<mapped_type> V>
	typename Table::insert_result insert_or_assign(K&& key, V&& value)
	{
		auto const r = Table::insert(vsm_as_const(key));

		if (r.inserted)
		{
#ifndef __INTELLISENSE__
			::new (std::to_address(r.iterator)) element_type(
				vsm_forward(key),
				vsm_forward(value));
#endif
		}
		else
		{
			r.iterator->value = vsm_forward(value);
		}

		return r;
	}


	template<detail::hash_table_key<Table> K>
	size_t erase(K const& key)
	{
		return Table::erase(key);
	}

	ordered_iterator_or_void erase(typename Table::const_single_iterator const position)
	{
		return Table::erase(position);
	}
};

} // namespace vsm
