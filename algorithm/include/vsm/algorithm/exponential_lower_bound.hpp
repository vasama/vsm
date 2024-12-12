#pragma once

#include <vsm/utility.hpp>

#include <algorithm>
#include <functional>

namespace vsm {

template<
	std::random_access_iterator Iterator,
	typename T = std::iter_value_t<Iterator>,
	typename Comparator>
[[nodiscard]] constexpr Iterator exponential_lower_bound(
	Iterator begin,
	Iterator end,
	T const& value,
	Comparator comparator)
{
	using difference_type = std::iter_difference_t<Iterator>;

	if (begin == end)
	{
		return end;
	}

	Iterator middle = begin;
	difference_type step = static_cast<difference_type>(1);
	difference_type left = end - begin;

	while (true)
	{
		if (!vsm_as_const(comparator)(*middle, value))
		{
			return std::lower_bound(
				vsm_move(begin),
				vsm_move(middle),
				value,
				vsm_move(comparator));
		}
		begin = vsm_move(middle);

		if (step >= left / 2)
		{
			return std::lower_bound(
				vsm_move(begin),
				vsm_move(end),
				value,
				vsm_move(comparator));
		}
		middle = begin + step;

		left -= step;
		step *= 2;
	}
}

template<
	std::random_access_iterator Iterator,
	typename T = std::iter_value_t<Iterator>>
[[nodiscard]] constexpr Iterator exponential_lower_bound(
	Iterator begin,
	Iterator end,
	T const& value)
{
	return exponential_lower_bound(vsm_move(begin), vsm_move(end), value, std::less<>());
}

template<
	std::random_access_iterator Iterator,
	typename T = std::iter_value_t<Iterator>,
	typename Comparator>
[[nodiscard]] constexpr Iterator exponential_lower_bound_near(
	Iterator begin,
	Iterator near,
	Iterator end,
	T const& value,
	Comparator comparator)
{
	if (near == end || vsm_as_const(comparator)(value, *near))
	{
		constexpr auto make_reverse_comparator = [](Comparator&& comparator)
		{
			return [c = vsm_move(comparator)](auto const& lhs, auto const& rhs)
			{
				return !c(lhs, rhs);
			};
		};

		return exponential_lower_bound(
			std::reverse_iterator(vsm_move(near)),
			std::reverse_iterator(vsm_move(begin)),
			value,
			make_reverse_comparator(vsm_move(comparator))).base();
	}
	else
	{
		return exponential_lower_bound(
			vsm_move(near),
			vsm_move(end),
			value,
			vsm_move(comparator));
	}
}

template<
	std::random_access_iterator Iterator,
	typename T = std::iter_value_t<Iterator>>
[[nodiscard]] constexpr Iterator exponential_lower_bound_near(
	Iterator begin,
	Iterator near,
	Iterator end,
	T const& value)
{
	return exponential_lower_bound_near(
		vsm_move(begin),
		vsm_move(near),
		vsm_move(end),
		value,
		std::less<>());
}

} // namespace vsm
