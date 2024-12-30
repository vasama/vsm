#include <vsm/numeric.hpp>

using namespace vsm;

namespace {

using sc = signed char;
using uc = unsigned char;

using ss = signed short;
using us = unsigned short;

using si = signed int;
using ui = unsigned int;

using sl = signed long;
using ul = unsigned long;


template<std::signed_integral T>
static constexpr T neg = -1;

template<typename T>
static constexpr T nul = 0;

template<typename T>
static constexpr T one = 1;

template<typename T>
static constexpr T max = std::numeric_limits<T>::max();

template<std::signed_integral T>
static constexpr T min = std::numeric_limits<T>::min();

template<typename T>
static constexpr T msb = most_significant_bit<T>;

template<typename T>
static constexpr bool cmp(T const expected, std::same_as<T> auto const value)
{
	return expected == value;
}


/* saturate */

static_assert(saturate<ss>(max<sl>) == max<ss>);
static_assert(saturate<ss>(min<sl>) == min<ss>);

static_assert(saturate<us>(max<sl>) == max<us>);
static_assert(saturate<us>(min<sl>) == 0);

static_assert(saturate<si>(max<ui>) == max<si>);
static_assert(saturate<si>(nul<ui>) == 0);

static_assert(saturate<ui>(max<si>) == max<si>);
static_assert(saturate<ui>(min<si>) == 0);

/* truncate */

/* unsigned int -> signed int */
static_assert(    try_truncate<si>(nul<ui>));
static_assert(    try_truncate<si>(one<ui>));
static_assert(    try_truncate<si>(static_cast<ui>(max<si>)));
static_assert(not try_truncate<si>(static_cast<ui>(max<si>) + one<ui>));
static_assert(not try_truncate<si>(msb<ui>));
static_assert(not try_truncate<si>(max<ui>));

/* signed int -> unsigned int */
static_assert(not try_truncate<ui>(min<si>));
static_assert(    try_truncate<ui>(nul<si>));
static_assert(    try_truncate<ui>(one<si>));
static_assert(    try_truncate<ui>(max<si>));

/* unsigned int -> signed char */
static_assert(    try_truncate<sc>(nul<ui>));
static_assert(    try_truncate<sc>(one<ui>));
static_assert(    try_truncate<sc>(static_cast<ui>(max<sc>)));
static_assert(not try_truncate<sc>(static_cast<ui>(max<sc>) + one<ui>));
static_assert(not try_truncate<sc>(msb<ui>));
static_assert(not try_truncate<sc>(max<ui>));

/* signed char -> unsigned int */
static_assert(not try_truncate<ui>(min<sc>));
static_assert(not try_truncate<ui>(neg<sc>));
static_assert(    try_truncate<ui>(nul<sc>));
static_assert(    try_truncate<ui>(one<sc>));
static_assert(    try_truncate<ui>(max<sc>));

/* signed int -> signed char */
static_assert(not try_truncate<sc>(min<si>));
static_assert(    try_truncate<sc>(neg<si>));
static_assert(    try_truncate<sc>(nul<si>));
static_assert(    try_truncate<sc>(one<si>));
static_assert(    try_truncate<sc>(static_cast<si>(min<sc>)));
static_assert(    try_truncate<sc>(static_cast<si>(max<sc>)));
static_assert(not try_truncate<sc>(static_cast<si>(max<sc>) + one<si>));
static_assert(not try_truncate<sc>(max<si>));

/* signed char -> signed int */
static_assert(    try_truncate<si>(min<sc>));
static_assert(    try_truncate<si>(neg<sc>));
static_assert(    try_truncate<si>(nul<sc>));
static_assert(    try_truncate<si>(one<sc>));
static_assert(    try_truncate<si>(max<sc>));

/* unsigned int -> unsigned char */
static_assert(    try_truncate<uc>(nul<ui>));
static_assert(    try_truncate<uc>(one<ui>));
static_assert(    try_truncate<uc>(static_cast<ui>(max<uc>)));
static_assert(not try_truncate<uc>(static_cast<ui>(max<uc>) + one<ui>));
static_assert(not try_truncate<uc>(max<ui>));

/* unsigned char -> unsigned int */
static_assert(    try_truncate<ui>(nul<uc>));
static_assert(    try_truncate<ui>(one<uc>));
static_assert(    try_truncate<ui>(msb<uc>));
static_assert(    try_truncate<ui>(max<uc>));

/* signed int -> unsigned char */
static_assert(not try_truncate<uc>(min<si>));
static_assert(not try_truncate<uc>(neg<si>));
static_assert(    try_truncate<uc>(nul<si>));
static_assert(    try_truncate<uc>(static_cast<si>(max<uc>)));
static_assert(not try_truncate<uc>(static_cast<si>(max<uc>) + one<si>));
static_assert(not try_truncate<uc>(max<si>));

/* unsigned char -> signed int */
static_assert(    try_truncate<si>(nul<uc>));
static_assert(    try_truncate<si>(one<uc>));
static_assert(    try_truncate<si>(msb<uc>));
static_assert(    try_truncate<si>(max<uc>));


/* min, max */

static_assert(cmp<si>(
	-7,
	vsm::min(static_cast<si>(42), static_cast<ss>(-7))));

static_assert(cmp<si>(
	42,
	vsm::max(static_cast<si>(42), static_cast<ss>(-7))));

static_assert(cmp<sl>(
	-7,
	vsm::min(static_cast<si>(42), static_cast<sl>(-7))));

static_assert(cmp<sl>(
	42,
	vsm::max(static_cast<si>(42), static_cast<sl>(-7))));

static_assert(cmp<ui>(
	17,
	vsm::min(static_cast<ui>(42), static_cast<us>(17))));

static_assert(cmp<ui>(
	42,
	vsm::max(static_cast<ui>(42), static_cast<us>(17))));

static_assert(cmp<ul>(
	17,
	vsm::min(static_cast<ui>(42), static_cast<ul>(17))));

static_assert(cmp<ul>(
	42,
	vsm::max(static_cast<ui>(42), static_cast<ul>(17))));


/* powers of two */

static_assert(most_significant_bit<uint8_t> == static_cast<uint8_t>(1) << 7);
static_assert(most_significant_bit<uint16_t> == static_cast<uint16_t>(1) << 15);
static_assert(most_significant_bit<uint32_t> == static_cast<uint32_t>(1) << 31);
static_assert(most_significant_bit<uint64_t> == static_cast<uint64_t>(1) << 63);

static_assert(is_power_of_two_or_zero(0u));
static_assert(is_power_of_two_or_zero(1u));
static_assert(is_power_of_two_or_zero(8u));
static_assert(is_power_of_two_or_zero(msb<ui>));
static_assert(!is_power_of_two_or_zero(msb<ui> | 1u));
static_assert(!is_power_of_two_or_zero(max<ui>));

static_assert(po2_ceil(0u, 1u) == 0u);
static_assert(po2_ceil(1u, 1u) == 1u);
static_assert(po2_ceil(2u, 1u) == 2u);
static_assert(po2_ceil(0u, 8u) == 0u);
static_assert(po2_ceil(1u, 8u) == 8u);
static_assert(po2_ceil(8u, 8u) == 8u);
static_assert(po2_ceil(9u, 8u) == 16u);
static_assert(po2_ceil(msb<ui> - 1u, 1u) == msb<ui> - 1u);
static_assert(po2_ceil(msb<ui> - 1u, 2u) == msb<ui>);
static_assert(po2_ceil(msb<ui> - 1u, msb<ui>) == msb<ui>);
static_assert(po2_ceil(msb<ui>, 1u) == msb<ui>);
static_assert(po2_ceil(msb<ui>, 2u) == msb<ui>);
static_assert(po2_ceil(msb<ui>, msb<ui>) == msb<ui>);

static_assert(po2_floor(0u, 1u) == 0u);
static_assert(po2_floor(1u, 1u) == 1u);
static_assert(po2_floor(2u, 1u) == 2u);
static_assert(po2_floor(0u, 8u) == 0u);
static_assert(po2_floor(7u, 8u) == 0u);
static_assert(po2_floor(8u, 8u) == 8u);
static_assert(po2_floor(9u, 8u) == 8u);
static_assert(po2_floor(msb<ui> - 1u, 1u) == msb<ui> - 1u);
static_assert(po2_floor(msb<ui> - 1u, 8u) == msb<ui> - 8u);
static_assert(po2_floor(msb<ui> - 1u, msb<ui>) == 0u);
static_assert(po2_floor(msb<ui>, msb<ui>) == msb<ui>);
static_assert(po2_floor(msb<ui> + 1u, msb<ui>) == msb<ui>);
static_assert(po2_floor(max<ui>, msb<ui>) == msb<ui>);

} // namespace
