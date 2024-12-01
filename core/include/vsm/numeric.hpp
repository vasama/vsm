#pragma once

#include <vsm/assert.h>
#include <vsm/concepts.hpp>

#include <bit>
#include <expected>
#include <limits>
#include <utility>

namespace vsm {

template<std::integral From>
constexpr std::make_signed_t<From> to_signed(From const value)
{
	return static_cast<std::make_signed_t<From>>(value);
}

template<std::integral From>
constexpr std::make_unsigned_t<From> to_unsigned(From const value)
{
	return static_cast<std::make_unsigned_t<From>>(value);
}

template<vsm::enumeration Enum>
constexpr Enum to_enum(std::underlying_type_t<Enum> const underlying)
{
	return static_cast<Enum>(underlying);
}


namespace detail {

struct signed_integral_t;
struct unsigned_integral_t;
struct floating_point_t;

void _arithmetic_category(auto const&) = delete;
signed_integral_t _arithmetic_category(std::signed_integral auto const&);
unsigned_integral_t _arithmetic_category(std::unsigned_integral auto const&);
floating_point_t _arithmetic_category(std::floating_point auto const&);

template<typename T>
using arithmetic_category_t = decltype(_arithmetic_category(std::declval<T>()));

template<typename L, typename R>
inline constexpr bool is_same_arithmetic_category_v =
	std::is_same_v<arithmetic_category_t<L>, arithmetic_category_t<R>>;


template<std::integral To, std::integral From>
consteval bool _may_lose_precision()
{
	if constexpr (std::signed_integral<From>)
	{
		if constexpr (std::unsigned_integral<To>)
		{
			return true;
		}
		else
		{
			// Both To and From are signed here:
			if (std::numeric_limits<To>::min() > std::numeric_limits<From>::min())
			{
				return true;
			}
		}
	}

	return
		to_unsigned(std::numeric_limits<To>::max()) <
		to_unsigned(std::numeric_limits<From>::max());
}

template<std::integral To, std::unsigned_integral From>
constexpr bool _loses_precision(From const from)
{
	if constexpr (to_unsigned(std::numeric_limits<To>::max()) < std::numeric_limits<From>::max())
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
	if constexpr (std::numeric_limits<To>::max() >= to_unsigned(std::numeric_limits<From>::max()))
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
constexpr To saturate(From const from)
{
	if constexpr (to_unsigned(std::numeric_limits<To>::max()) >= std::numeric_limits<From>::max())
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
constexpr To saturate(From const from)
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
constexpr To saturate(From const from)
{
	if constexpr (std::numeric_limits<To>::max() >= to_unsigned(std::numeric_limits<From>::max()))
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
	explicit constexpr saturating(From const from)
		: m_from(from)
	{
	}

	template<std::integral To>
	constexpr operator To() const
	{
		return saturate<To>(m_from);
	}
};


template<std::integral To, std::integral From>
constexpr To truncate(From const from)
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
	explicit constexpr truncating(From const from)
		: m_from(from)
	{
	}

	template<std::integral To>
	constexpr operator To() const
	{
		return truncate<To>(m_from);
	}
};


template<std::integral To, std::integral From, typename Error = loses_precision_t>
constexpr std::expected<To, Error> try_truncate(
	From const from,
	Error const& error = loses_precision_t())
{
	if constexpr (detail::_may_lose_precision<To, From>())
	{
		if (detail::_loses_precision<To>(from))
		{
			return std::unexpected(error);
		}
	}
	return static_cast<To>(from);
}


template<arithmetic L, arithmetic R>
constexpr auto min(L const l, R const r) -> decltype(l + r)
	requires detail::is_same_arithmetic_category_v<L, R>
{
	return l <= r ? l : r;
}

template<arithmetic L, arithmetic R>
constexpr auto max(L const l, R const r) -> decltype(l + r)
	requires detail::is_same_arithmetic_category_v<L, R>
{
	return l >= r ? l : r;
}

template<arithmetic T, arithmetic Min, arithmetic Max>
constexpr auto clamp(T const x, Min const min, Max const max) -> decltype (x + (min + max))
	requires
		detail::is_same_arithmetic_category_v<T, Min> &&
		detail::is_same_arithmetic_category_v<T, Max>
{
	using type = decltype(x + (min + max));
	vsm_assert_slow(min < max);
	if (x < min) return static_cast<type>(min);
	if (x > max) return static_cast<type>(max);
	return static_cast<type>(x);
}


template<std::unsigned_integral T>
inline constexpr T most_significant_bit = static_cast<T>(~(static_cast<T>(-1) >> 1));

template<std::unsigned_integral T>
constexpr bool is_power_of_two_or_zero(T const value)
{
	return (value & (value - 1)) == 0;
}

template<std::unsigned_integral T>
[[deprecated("Use std::has_single_bit instead")]]
constexpr bool is_power_of_two(T const value)
{
	return std::has_single_bit(value);
}

template<std::unsigned_integral T>
[[deprecated("Use std::bit_ceil instead")]]
constexpr T round_up_to_power_of_two(T const value)
{
	return std::bit_ceil(value);
}

template<std::unsigned_integral T>
constexpr T po2_ceil(T const value, T const power_of_two)
{
	vsm_assert_slow(std::has_single_bit(power_of_two));
	T const power_of_two_bits = power_of_two - 1;
	return value + power_of_two_bits & ~power_of_two_bits;
}

template<std::unsigned_integral T>
[[deprecated("Use vsm::po2_ceil instead")]]
constexpr T round_up_to_power_of_two(T const value, T const power_of_two)
{
	return po2_ceil(value, power_of_two);
}

template<std::unsigned_integral T>
constexpr T po2_floor(T const value, T const power_of_two)
{
	vsm_assert_slow(std::has_single_bit(power_of_two));
	return value - (value & (power_of_two - 1));
}

template<std::unsigned_integral T>
[[deprecated("Use vsm::po2_floor instead")]]
constexpr T round_down_to_power_of_two(T const value, T const power_of_two)
{
	return po2_floor(value, power_of_two);
}

} // namespace vsm
