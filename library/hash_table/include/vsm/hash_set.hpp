#pragma once

#include <vsm/concepts.hpp>
#include <vsm/default_hash.hpp>
#include <vsm/detail/hash_table.hpp>
#include <vsm/standard/stdexcept.hpp>

#include <vsm/detail/swiss_table.hpp>

#include <memory>
#include <new>

namespace vsm {

template<non_cvref Table>
class basic_hash_set : public Table
{
public:
	using element_type          = typename Table::element_type;
	using key_type              = element_type;
	using value_type            = element_type;

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
		typename Table::insert_result r = Table::insert(vsm_as_const(key));

		if (r.inserted)
		{
			::new (std::to_address(r.iterator)) element_type(vsm_forward(key));
		}

		return r;
	}

	template<detail::hash_table_key<Table> K>
	typename Table::insert_result insert_or_assign(K&& key)
	{
		typename Table::insert_result r = Table::insert(vsm_as_const(key));

		if (r.inserted)
		{
			::new (std::to_address(r.iterator)) element_type(vsm_forward(key));
		}
		else
		{
			*r.iterator = vsm_forward(key);
		}

		return r;
	}

	template<typename... Args>
		requires std::constructible_from<value_type, Args...>
	typename Table::insert_result emplace(Args&&... args)
	{
		element_type new_element(vsm_forward(args)...);
	
		typename Table::insert_result r = Table::insert(vsm_as_const(new_element));

		if (r.inserted)
		{
			::new (std::to_address(r.iterator)) element_type(vsm_move(new_element));
		}

		return r;
	}

	template<typename... Args>
	typename Table::insert_result emplace_or_assign(Args&&... args)
	{
		element_type new_element(vsm_forward(args)...);

		typename Table::insert_result r = Table::insert(vsm_as_const(new_element));

		if (r.inserted)
		{
			::new (std::to_address(r.iterator)) element_type(vsm_move(new_element));
		}
		else
		{
			*r.iterator = vsm_move(new_element);
		}

		return r;
	}


	template<detail::hash_table_key<Table> K>
	bool erase(K const& key)
	{
		return Table::erase(key);
	}

	ordered_iterator_or_void erase(typename Table::const_single_iterator const position)
	{
		return Table::erase(position);
	}
};

} // namespace vsm
