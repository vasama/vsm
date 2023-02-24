#pragma once

#include <vsm/standard.hpp>

#include <type_traits>

namespace vsm::detail::std_table_ {

template<typename Policies>
struct std_hash
{
	vsm_static_operator size_t operator()(auto const& key) vsm_static_operator_const
	{
		static constexpr Policies p = {};
		return p.hash(p.select(key));
	}
};

template<typename Policies>
struct std_compare
{
	vsm_static_operator size_t operator()(auto const& lhs, auto const& rhs) vsm_static_operator_const
	{
		static constexpr Policies p = {};
		return p.compare(p.select(lhs), p.select(rhs));
	}
};

template<typename Allocator, typename T>
class std_allocator
{
	vsm_no_unique_address Allocator m_allocator;

public:
	T* allocate(size_t const count)
	{
		return static_cast<T*>(m_allocator.allocate(count * sizeof(T)).buffer);
	}

	void deallocate(T* const block, size_t const count)
	{
		return m_allocator.deallocate(allocation(block, count * sizeof(T)));
	}
}

template<typename T, typename Key, typename Policies, typename Allocator, size_t Capacity>
class table : std::unordered_set<T, std_hash<Policies>, std_compare<Policies>, std_allocator<Allocator>>
{
	using b = std::unordered_set<T, std_hash<Policies>, std_compare<Policies>, std_allocator<Allocator>>;

	using iterator = typename std_set::iterator;
	using const_iterator = typename std_set::const_iterator;

public:
	using b::empty;
	using b::size;
	using b::capacity;
	using b::reserve;
	using b::shrink_to_fit;

	
};

} // namespace vsm::detail::std_table_
