#pragma once

#include <vsm/utility.hpp>

#include <algorithm>

namespace vsm {

template<
	std::random_access_iterator Iterator,
	std::sized_sentinel_for<Iterator> Sentinel,
	typename T = std::iter_value_t<Iterator>,
	typename Comparator>
[[nodiscard]] constexpr Iterator exponential_lower_bound(
	Iterator begin,
	Sentinel end,
	T const& value,
	Comparator comparator)
{
	using difference_type = std::iter_difference_t<Iterator>;

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
		middle = middle + step;

		left -= step;
		step *= 2;
	}
}

template<
	std::random_access_iterator Iterator,
	std::sized_sentinel_for<Iterator> Sentinel,
	typename T = std::iter_value_t<Iterator>,
	typename Comparator>
[[nodiscard]] constexpr Iterator lower_bound_near(
	Iterator begin,
	Iterator near,
	Sentinel end,
	T const& value,
	Comparator comparator)
{
	if (near != end && vsm_as_const(comparator)(*near, value))
	{
		return exponential_lower_bound(
			std::reverse_iterator(vsm_move(near)),
			std::reverse_iterator(vsm_move(begin)),
			value,
			[c = vsm_move(comparator)](auto const&... args) { return c(args...); }).base();
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

} // namespace vsm
