#pragma once

#include <vsm/detail/span.hpp>
#include <vsm/concepts.hpp>
#include <vsm/hash.hpp>

#include <algorithm>
#include <compare>

namespace vsm {

template<non_cvref T>
class array_view;

template<typename View, vsm::non_cvref T>
class array_view_base : public detail::_span<View, T const>
{
public:
	using detail::_span<View, T const>::_span;

	[[nodiscard]] friend constexpr bool operator==(View const& lhs, View const& rhs)
	{
		return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}

	[[nodiscard]] friend constexpr auto operator<=>(View const& lhs, View const& rhs)
	{
		return std::lexicographical_compare_three_way(
			lhs.begin(),
			lhs.end(),
			rhs.begin(),
			rhs.end());
	}
};

template<non_cvref T>
class array_view : public array_view_base<array_view<T>, T>
{
public:
	using array_view_base<array_view<T>, T>::array_view_base;

	template<typename State>
	friend void tag_invoke(decltype(hash_append), State& state, array_view const& view)
	{
		hash_append(state, view.begin(), view.end());
	}
};

} // namespace vsm
