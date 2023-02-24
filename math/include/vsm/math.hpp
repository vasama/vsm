#pragma once

#include <vsm/assert.h>
#include <vsm/concepts.hpp>

#include <bit>
#include <type_traits>

namespace vsm {
namespace detail {

template<typename>
struct arithmetic_category;

template<std::signed_integral T>
struct arithmetic_category<T>
{
	static constexpr int value = 1;
};

template<std::unsigned_integral T>
struct arithmetic_category<T>
{
	static constexpr int value = 2;
};

template<std::floating_point T>
struct arithmetic_category<T>
{
	static constexpr int value = 3;
};

template<typename L, typename R>
inline constexpr bool is_same_arithmetic_category =
	arithmetic_category<L>::value == arithmetic_category<R>::value;

} // namespace detail

template<std::unsigned_integral T>
inline constexpr T most_significant_bit = ~(static_cast<T>(-1) >> 1);


template<arithmetic L, arithmetic R>
constexpr auto min(L const l, R const r) -> decltype(l + r)
	requires detail::is_same_arithmetic_category<L, R>
{
	return l <= r ? l : r;
}

template<arithmetic L, arithmetic R>
constexpr auto max(L const l, R const r) -> decltype(l + r)
	requires detail::is_same_arithmetic_category<L, R>
{
	return l >= r ? l : r;
}

template<arithmetic T, arithmetic Min, arithmetic Max>
constexpr auto clamp(T const x, Min const min, Max const max) -> decltype (x + (min + max))
	requires
		detail::is_same_arithmetic_category<T, Min> &&
		detail::is_same_arithmetic_category<T, Max>
{
	using type = decltype(x + (min + max));
	vsm_assert_slow(min < max);
	if (x < min) return static_cast<type>(min);
	if (x > max) return static_cast<type>(max);
	return static_cast<type>(x);
}


template<std::unsigned_integral T>
constexpr bool is_power_of_two_or_zero(T const value)
{
	return (value & (value - 1)) == 0;
}

template<std::unsigned_integral T>
constexpr bool is_power_of_two(T const value)
{
	return std::has_single_bit(value);
}

template<std::unsigned_integral T>
constexpr T round_up_to_power_of_two(T const value)
{
	vsm_assert(value < most_significant_bit<T>);
	return most_significant_bit<T> >> (std::countl_zero(value) - 1);
}

template<std::unsigned_integral T>
constexpr T round_up_to_power_of_two(T const value, T const power_of_two)
{
	vsm_assert_slow(is_power_of_two(power_of_two));
	T const power_of_two_bits = power_of_two - 1;
	return value + power_of_two_bits & ~power_of_two_bits;
}

template<std::unsigned_integral T>
constexpr T round_down_to_power_of_two(T const value, T const power_of_two)
{
	return value - (value & (power_of_two - 1));
}

} // namespace vsm
