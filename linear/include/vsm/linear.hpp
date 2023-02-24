#pragma once

#include <type_traits>

namespace vsm {

template<typename T, auto DefaultValue = T{}>
struct linear
{
	static_assert(std::is_trivially_copyable_v<T>);

	T value = DefaultValue;

	linear() = default;

	constexpr linear(T const value) noexcept
		: value(value)
	{
	}

	constexpr linear(linear&& source) noexcept
		: value(source.value)
	{
		source.value = DefaultValue;
	}

	constexpr linear& operator=(linear&& source) noexcept
	{
		value = source.value;
		source.value = DefaultValue;
		return *this;
	}

	constexpr void reset() noexcept
	{
		value = DefaultValue;
	}
};

} // namespace vsm
