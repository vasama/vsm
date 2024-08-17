#pragma once

#include <vsm/concepts.hpp>
#include <vsm/preprocessor.h>

#if __has_include(vsm_pp_include(vsm/detail/float16/vsm_arch.h))
#	include vsm_pp_include(vsm/detail/float16/vsm_arch.h)
#else
#	include vsm_pp_include(vsm/detail/float16/generic.h)
#endif

#include <compare>

#include <cstdint>

namespace vsm::detail {

class _float16;

} // namespace vsm::detail

template<std::floating_point Float>
struct std::common_type<vsm::detail::_float16, Float>
{
	using type = Float;
};

template<std::floating_point Float>
struct std::common_type<Float, vsm::detail::_float16>
{
	using type = Float;
};

template<std::integral Integral>
struct std::common_type<Integral, vsm::detail::_float16>
{
	using type = float;
};

template<std::integral Integral>
struct std::common_type<vsm::detail::_float16, Integral>
{
	using type = float;
};

namespace vsm::detail {

template<typename>
concept _float16_explicit = true;

template<typename Float>
concept _float16_concept = std::floating_point<Float> || std::same_as<Float, _float16>;

template<typename L, typename R>
using _float16_promote = std::common_type_t<
#if vsm_detail_half_promotion
	float,
#endif
	L,
	R>;

template<typename T, typename U>
concept _losslessly_convertible_to =
	std::convertible_to<T, U> &&
	requires (T const& t)
	{
		U{ t };
	};

class _float16 final
{
	static constexpr uint16_t sign              = 0x8000;
	static constexpr uint16_t magnitude         = 0x7fff;

	uint16_t m_value;

public:
	template<not_same_as<_float16> T, int = 0>
		requires std::convertible_to<T, float>
	[[deprecated("Conversion from T to float16_t loses precision.")]]
	_float16(T const& value)
		noexcept(noexcept(static_cast<float>(value)))
		: m_value(vsm_detail_float_to_half(value))
	{
	}

	template<not_same_as<_float16> T, int = 1>
		requires std::convertible_to<T, float> && _float16_explicit<T>
	explicit _float16(T const& value)
		noexcept(noexcept(static_cast<float>(value)))
		: m_value(vsm_detail_float_to_half(value))
	{
	}


	template<not_same_as<_float16> T, int = 0>
		requires std::convertible_to<float, T>
	[[deprecated("Conversion from float16_t to T loses precision.")]]
	[[nodiscard]] operator T() const
		noexcept(noexcept(static_cast<T>(0.0f)))
	{
		return static_cast<T>(vsm_detail_half_to_float(m_value));
	}

	template<not_same_as<_float16> T, int = 1>
		requires _losslessly_convertible_to<float, T>
	[[nodiscard]] operator T() const
		noexcept(noexcept(static_cast<T>(0.0f)))
	{
		return static_cast<T>(vsm_detail_half_to_float(m_value));
	}

	template<not_same_as<_float16> T, int = 2>
		requires std::convertible_to<float, T> && _float16_explicit<T>
	[[nodiscard]] explicit operator T() const
		noexcept(noexcept(static_cast<T>(0.0f)))
	{
		return static_cast<T>(vsm_detail_half_to_float(m_value));
	}

	template<not_same_as<_float16> T, int = 3>
		requires _losslessly_convertible_to<float, T> && _float16_explicit<T>
	[[nodiscard]] explicit operator T() const
		noexcept(noexcept(static_cast<T>(0.0f)))
	{
		return static_cast<T>(vsm_detail_half_to_float(m_value));
	}


	[[nodiscard]] friend _float16 operator+(_float16 const value)
	{
		return value;
	}

	[[nodiscard]] friend _float16 operator-(_float16 const value)
	{
		return _float16(static_cast<uint16_t>(value.m_value ^ sign));
	}

	template<_float16_concept L, _float16_concept R>
	[[nodiscard]] friend _float16_promote<L, R> operator+(L const lhs, R const rhs)
	{
		return static_cast<_float16_promote<L, R>>(
			vsm_detail_half_to_float(lhs.m_value) + vsm_detail_half_to_float(rhs.m_value));
	}

	template<_float16_concept L, _float16_concept R>
	[[nodiscard]] friend _float16_promote<L, R> operator-(L const lhs, R const rhs)
	{
		return static_cast<_float16_promote<L, R>>(
			vsm_detail_half_to_float(lhs.m_value) - vsm_detail_half_to_float(rhs.m_value));
	}

	template<_float16_concept L, _float16_concept R>
	[[nodiscard]] friend _float16_promote<L, R> operator*(L const lhs, R const rhs)
	{
		return static_cast<_float16_promote<L, R>>(
			vsm_detail_half_to_float(lhs.m_value) * vsm_detail_half_to_float(rhs.m_value));
	}

	template<_float16_concept L, _float16_concept R>
	[[nodiscard]] friend _float16_promote<L, R> operator/(L const lhs, R const rhs)
	{
		return static_cast<_float16_promote<L, R>>(
			vsm_detail_half_to_float(lhs.m_value) / vsm_detail_half_to_float(rhs.m_value));
	}

	[[nodiscard]] friend bool operator==(_float16 const lhs, _float16 const rhs)
	{
		bool const e = lhs.m_value == rhs.m_value;
		bool const z = ((lhs.m_value | rhs.m_value) & magnitude) == 0;
		return e | z;
	}

	template<_float16_concept L, _float16_concept R>
	[[nodiscard]] friend bool operator==(L const lhs, R const rhs)
	{
		return vsm_detail_half_to_float(lhs.m_value) == vsm_detail_half_to_float(rhs.m_value);
	}

	[[nodiscard]] friend bool operator!=(_float16 const lhs, _float16 const rhs)
	{
		bool const e = lhs.m_value != rhs.m_value;
		bool const z = ((lhs.m_value | rhs.m_value) & magnitude) != 0;
		return e & z;
	}

	template<_float16_concept L, _float16_concept R>
	[[nodiscard]] friend bool operator!=(L const lhs, R const rhs)
	{
		return vsm_detail_half_to_float(lhs.m_value) != vsm_detail_half_to_float(rhs.m_value);
	}

	template<_float16_concept L, _float16_concept R>
	[[nodiscard]] friend auto operator<=>(L const lhs, R const rhs)
	{
		return vsm_detail_half_to_float(lhs.m_value) <=> vsm_detail_half_to_float(rhs.m_value);
	}

private:
	explicit _float16(uint16_t const value)
		: m_value(value)
	{
	}
};

} // namespace vsm::detail

#define vsm_detail_float16 detail::_float16
