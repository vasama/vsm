#pragma once

#include <vsm/utility.hpp>

#include <iterator>

namespace vsm {

template<std::bidirectional_iterator Iterator>
constexpr Iterator remove_unstable(Iterator beg, Iterator end, auto const& value)
{
	while (true)
	{
		while (true)
		{
			if (beg == end)
			{
				return beg;
			}

			if (vsm_as_const(*beg) == value)
			{
				break;
			}

			++beg;
		}

		while (true)
		{
			if (beg == --end)
			{
				return beg;
			}

			if (vsm_as_const(*end) != value)
			{
				break;
			}
		}

		*beg++ = vsm_move(*end);
	}
}

template<std::bidirectional_iterator Iterator>
constexpr Iterator remove_if_unstable(Iterator beg, Iterator end, auto&& pred)
{
	while (true)
	{
		while (true)
		{
			if (beg == end)
			{
				return beg;
			}

			if (pred(*beg))
			{
				break;
			}

			++beg;
		}

		while (true)
		{
			if (beg == --end)
			{
				return beg;
			}

			if (!pred(*end))
			{
				break;
			}
		}

		*beg++ = vsm_move(*end);
	}
}

} // namespace vsm
