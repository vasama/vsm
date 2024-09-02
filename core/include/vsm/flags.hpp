#pragma once

#include <type_traits>

namespace vsm {

template<typename T>
[[nodiscard]] constexpr bool no_flags(T const superset, T const subset)
{
	return (superset & subset) == T{};
}

template<typename T>
[[nodiscard]] constexpr bool any_flags(T const superset, T const subset)
{
	return (superset & subset) != T{};
}

template<typename T>
[[nodiscard]] constexpr bool all_flags(T const superset, T const subset)
{
	return (superset & subset) == subset;
}

template<typename T>
[[nodiscard]] constexpr T set_flags(T const superset, T const subset, bool const value)
{
	return (superset & ~subset) | (subset & static_cast<T>(-value));
}

} // namespace vsm

#define vsm_detail_flag_enum_u(T, O, ...) \
	[[maybe_unused, nodiscard]] inline constexpr __VA_ARGS__ T operator O(T const r) \
	{ \
		using U = ::std::underlying_type_t<T>; \
		return static_cast<T>(O static_cast<U>(r)); \
	}

#define vsm_detail_flag_enum_b(T, O, ...) \
	[[maybe_unused, nodiscard]] inline constexpr __VA_ARGS__ T operator O(T const l, T const r) \
	{ \
		using U = ::std::underlying_type_t<T>; \
		return static_cast<T>(static_cast<U>(l) O static_cast<U>(r)); \
	}

#define vsm_detail_flag_enum_a(T, O, ...) \
	[[maybe_unused]] inline constexpr __VA_ARGS__ T& operator O ## =(T& l, T const r) \
	{ \
		using U = ::std::underlying_type_t<T>; \
		return l = static_cast<T>(static_cast<U>(l) O static_cast<U>(r)); \
	}

#define vsm_detail_flag_enum(T, ...) \
	vsm_detail_flag_enum_u(T, ~ __VA_OPT__(, __VA_ARGS__)) \
	vsm_detail_flag_enum_b(T, & __VA_OPT__(, __VA_ARGS__)) \
	vsm_detail_flag_enum_a(T, & __VA_OPT__(, __VA_ARGS__)) \
	vsm_detail_flag_enum_b(T, | __VA_OPT__(, __VA_ARGS__)) \
	vsm_detail_flag_enum_a(T, | __VA_OPT__(, __VA_ARGS__)) \
	vsm_detail_flag_enum_b(T, ^ __VA_OPT__(, __VA_ARGS__)) \
	vsm_detail_flag_enum_a(T, ^ __VA_OPT__(, __VA_ARGS__)) \

#define vsm_flag_enum(T) \
	vsm_detail_flag_enum(T)

#define vsm_flag_enum_friend(T) \
	vsm_detail_flag_enum(T, friend)
