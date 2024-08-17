#pragma once

#include <vsm/assert.h>
#include <vsm/result.hpp>

#include <concepts>
#include <limits>
#include <utility>

namespace vsm {
namespace detail {

template<std::integral To, std::integral From>
consteval bool _may_lose_precision()
{
	if (std::signed_integral<From>)
	{
		if (std::unsigned_integral<To>)
		{
			return true;
		}

		if (std::numeric_limits<To>::min() > std::numeric_limits<From>::min())
		{
			return true;
		}
	}
	return std::numeric_limits<To>::max() < std::numeric_limits<From>::max();
}

template<std::integral To, std::unsigned_integral From>
constexpr bool _loses_precision(From const from)
{
	if constexpr (std::numeric_limits<To>::max() < std::numeric_limits<From>::max())
	{
		return from > static_cast<From>(std::numeric_limits<To>::max());
	}
	else
	{
		return false;
	}
}

template<std::signed_integral To, std::signed_integral From>
constexpr bool _loses_precision(From const from)
{
	if constexpr (
		std::numeric_limits<To>::min() > std::numeric_limits<From>::min() ||
		std::numeric_limits<To>::max() < std::numeric_limits<From>::max())
	{
		return
			from < static_cast<From>(std::numeric_limits<To>::min()) ||
			from > static_cast<From>(std::numeric_limits<To>::max());
	}
	else
	{
		return false;
	}
}

template<std::unsigned_integral To, std::signed_integral From>
constexpr bool _loses_precision(From const from)
{
	if constexpr (std::numeric_limits<To>::max() >= std::numeric_limits<From>::max())
	{
		return from < 0;
	}
	else
	{
		return
			from < 0 ||
			from > static_cast<From>(std::numeric_limits<To>::max());
	}
}

} // namespace detail

struct loses_precision_t {};


template<std::integral To, std::unsigned_integral From>
To saturate(From const from)
{
	if constexpr (std::numeric_limits<To>::max() >= std::numeric_limits<From>::max())
	{
		return static_cast<To>(from);
	}
	else
	{
		return static_cast<To>(std::min(
			from,
			static_cast<From>(std::numeric_limits<To>::max())));
	}
}

template<std::signed_integral To, std::signed_integral From>
To saturate(From const from)
{
	if constexpr (
		std::numeric_limits<To>::min() <= std::numeric_limits<From>::min() &&
		std::numeric_limits<To>::max() >= std::numeric_limits<From>::max())
	{
		return static_cast<To>(from);
	}
	else
	{
		return static_cast<To>(std::max(
			std::min(
				from,
				static_cast<From>(std::numeric_limits<To>::max())),
			static_cast<From>(std::numeric_limits<To>::min())));
	}
}

template<std::unsigned_integral To, std::signed_integral From>
To saturate(From const from)
{
	if constexpr (std::numeric_limits<To>::max() >= std::numeric_limits<From>::max())
	{
		return static_cast<To>(std::max(
			from,
			static_cast<From>(0)));
	}
	else
	{
		return static_cast<To>(std::max(
			std::min(
				from,
				static_cast<From>(std::numeric_limits<To>::max())),
			static_cast<From>(0)));
	}
}

template<std::integral From>
class saturating
{
	From m_from;

public:
	explicit saturating(From const from)
		: m_from(from)
	{
	}

	template<std::integral To>
	operator To() const
	{
		return saturate<To>(m_from);
	}
};


template<std::integral To, std::integral From>
To truncate(From const from)
{
	if constexpr (detail::_may_lose_precision<To, From>())
	{
		vsm_assert(!detail::_loses_precision<To>(from));
	}
	return static_cast<To>(from);
}

template<std::integral From>
class truncating
{
	From m_from;

public:
	explicit truncating(From const from)
		: m_from(from)
	{
	}

	template<std::integral To>
	operator To() const
	{
		return truncate<To>(m_from);
	}
};


template<std::integral To, std::integral From, typename Error = loses_precision_t>
vsm::result<To, Error> try_truncate(From const from, Error const& error = {})
{
	if constexpr (detail::_may_lose_precision<To, From>())
	{
		if (detail::_loses_precision<To>(from))
		{
			return vsm::unexpected(error);
		}
	}
	return static_cast<To>(from);
}

} // namespace vsm
