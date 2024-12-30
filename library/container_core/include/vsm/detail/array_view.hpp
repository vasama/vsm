#pragma once

#include <vsm/hash.hpp>

#include <algorithm>
#include <compare>
#include <span>

namespace vsm::detail {

template<typename T>
class array_view : std::span<T const>
{
	using span_type = std::span<T const>;

public:
	using element_type                  = T const;
	using value_type                    = T;
	using size_type                     = std::size_t;
	using difference_type               = std::ptrdiff_t;
	using pointer                       = T const*;
	using const_pointer                 = T const*;
	using reference                     = T const&;
	using const_reference               = T const&;
	using iterator                      = typename span_type::iterator;
	using const_iterator                = typename span_type::const_iterator;
	using reverse_iterator              = typename span_type::reverse_iterator;
	using const_reverse_iterator        = typename span_type::const_reverse_iterator;


	using span_type::span_type;

	using span_type::begin;
	using span_type::cbegin;
	using span_type::end;
	using span_type::cend;
	using span_type::rbegin;
	using span_type::crbegin;
	using span_type::rend;
	using span_type::crend;

	using span_type::front;
	using span_type::back;
#if __cpp_lib_span >= 202311L
	using span_type::at;
#endif
	using span_type::operator[];
	using span_type::data;

	using span_type::size;
	using span_type::size_bytes;
	using span_type::empty;


	[[nodiscard]] constexpr array_view first(size_type const count) const
	{
		return array_view(span_type::first(count));
	}

	[[nodiscard]] constexpr array_view last(size_type const count) const
	{
		return array_view(span_type::last(count));
	}

	[[nodiscard]] constexpr array_view subview(size_type const offset) const
	{
		return array_view(span_type::subspan(offset));
	}

	[[nodiscard]] constexpr array_view subview(size_type const offset, size_t const count) const
	{
		return array_view(span_type::subspan(offset, count));
	}


	[[nodiscard]] friend constexpr bool operator==(array_view const& lhs, array_view const& rhs)
	{
		return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}

	[[nodiscard]] friend constexpr auto operator<=>(array_view const& lhs, array_view const& rhs)
	{
		return std::lexicographical_compare_three_way(
			lhs.begin(),
			lhs.end(),
			rhs.begin(),
			rhs.end());
	}


	template<typename State>
	friend void tag_invoke(decltype(hash_append), State& state, array_view const& view)
	{
		hash_append(state, view.begin(), view.end());
	}
};

} // namespace vsm::detail
