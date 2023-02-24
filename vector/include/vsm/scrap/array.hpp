#pragma once

#include <vsm/default_allocator.hpp>
#include <vsm/standard.hpp>

namespace vsm {
namespace detail::array_ {

template<typename T, allocator A, size_t C>
class array
{
	static constexpr bool has_local_storage = C > 0;
	#define vsm_detail_array_capacity (C * sizeof(T))
	
	[[no_unique_address]] A m_allocator;

public:
	using value_type                                        = T;
	using allocator_type                                    = A;
	using size_type                                         = size_t;
	using difference_type                                   = ptrdiff_t;
	using reference                                         = T&;
	using const_reference                                   = T const&;
	using pointer                                           = T*;
	using const_pointer                                     = T const*;
	using iterator                                          = T*;
	using const_iterator                                    = T const*;


	constexpr vsm_always_inline array()
	{
		construct(m.c, vsm_detail_array_capacity);
	}
	
	explicit constexpr vsm_always_inline array(any_cvref_of<A> auto&& allocator)
		: m_allocator(vsm_forward(allocator))
	{
		construct(m.c, vsm_detail_array_capacity);
	}

	vsm_always_inline array(array&& other)
		: m_allocator(vsm_move(other.m_allocator))
	{
		vsm_detail_array::move_construct(
			m.c,
			other.m.c,
			m_allocator,
			vsm_detail_array_capacity,
			vsm_detail_array_capacity);
	}

	vsm_always_inline array& operator=(array&& other) &
	{
		vsm_detail_array::move_assign(
			m.c,
			other.m.c,
			m_allocator,
			vsm_move(other.m_allocator),
			vsm_detail_array_capacity,
			vsm_detail_array_capacity);
		return *this;
	}

	vsm_always_inline ~array()
	{
		vsm_detail_array::destroy(
			m.c,
			m_allocator,
			vsm_detail_array_capacity);
	}


	vsm_always_inline A const& get_allocator() const
	{
		return m_allocator;
	}
	
	
	bool empty() const
	{
		return m.c.beg == m.c.mid;
	}
	
	size_t size() const
	{
		return m.c.mid - m.c.beg;
	}
	
	size_t capacity() const
	{
		return m.c.end - m.c.beg;
	}
	
	
	template<std::convertible_to<T> U = T>
	T& push_back(U&& value)
	{
		void* const storage = *vsm_detail_array::push_back(
			m.c,
			sizeof(T),
			m_allocator,
			vsm_detail_array_capacity);
		return *new (storage) T(vsm_forward(value));
	}
	
	template<std::convertible_to<T> U = T>
	T* push_back_many(size_t const count, U const& value)
	{
		void* const storage = *vsm_detail_array::push_back(
			m.c,
			count * sizeof(T),
			m_allocator,
			vsm_detail_array_capacity);
		return std::uninitialized_construct_n(reinterpret_cast<T*>(storage), count, value);
	}
	
	template<std::ranges::range Range>
	T* push_back_range(Range&& range);

	template<std::convertible_to<T> U = T>
	T& insert(size_t const index, U&& value)
	{
		void* const storage = *vsm_detail_array::insert(
			m.c,
			index * sizeof(T),
			sizeof(T),
			m_allocator,
			vsm_detail_array_capacity);
		return *new (storage) T(vsm_forward(value));
	}
	
	template<std::convertible_to<T> U = T>
	T& insert_many(size_t const index, size_t const count, U const& value)
	{
		void* const storage = *vsm_detail_array::insert(
			m.c,
			index * sizeof(T),
			count * sizeof(T),
			m_allocator,
			vsm_detail_array_capacity);
		return std::uninitialized_construct_n(reinterpret_cast<T*>(storage), count, value);
	}
	
	template<std::ranges::range Range>
	T* insert_range(size_t const index, Range&& range);
	

	#undef vsm_detail_array_capacity
};

} // namespace detail::array_

} // namespace vsm
