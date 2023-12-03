#pragma once

#include <vsm/assert.h>
#include <vsm/result.hpp>

#include <concepts>
#include <limits>
#include <utility>

namespace vsm {

template<std::integral To, std::unsigned_integral From>
To saturate(From const from)
{
	if constexpr (std::numeric_limits<To>::max() >= std::numeric_limits<From>::max())
	{
		static_cast<To>(from);
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
		static_cast<To>(from);
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
	if constexpr (
		std::numeric_limits<To>::min() > std::numeric_limits<From>::min() ||
		std::numeric_limits<To>::max() < std::numeric_limits<From>::max())
	{
		vsm_assert(static_cast<From>(static_cast<To>(from)) == from);
	}
	return static_cast<To>(from);
}

template<std::integral To, std::integral From>
vsm::result<To, std::errc> try_truncate(From const from)
{
	if constexpr (
		std::numeric_limits<To>::min() > std::numeric_limits<From>::min() ||
		std::numeric_limits<To>::max() < std::numeric_limits<From>::max())
	{
		if ((static_cast<From>(static_cast<To>(from)) != from))
		{
			return vsm::unexpected(std::errc::value_too_large);
		}
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

} // namespace vsm
