#pragma once

#include <algorithm>
#include <ranges>
#include <span>

namespace vsm {

template<typename T>
class array_view : std::span<T const>
{
	using b = std::span<T const>;

public:
	friend constexpr bool operator==(array_view<T> const lhs, array_view<T> const rhs)
	{
		if (lhs.size() != rhs.size())
		{
			return false;
		}

		T const* const lhs_data = lhs.data();
		T const* const rhs_data = rhs.data();

		std::equal(lhs_data, lhs_data + lhs.size(), rhs_data, rhs_data + rhs.size());
	}

	friend constexpr auto operator<=>(array_view<T> const lhs, array_view<T> const rhs)
	{
		T const* const lhs_data = lhs.data();
		T const* const rhs_data = rhs.data();

		std::lexicographical_compare_three_way(lhs_data, lhs_data + lhs.size(), rhs_data, rhs_data + rhs.size());
	}
};

} // namespace vsm
