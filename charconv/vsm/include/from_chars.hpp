#pragma once

#include <vsm/result.hpp>

#include <charconv>
#include <concepts>

namespace vsm {

template<std::integral T>
vsm::result<T> from_chars(std::string_view const string, int const radix = 10)
{
	char const* const beg = string.data();
	char const* const end = string.data() + string.size();

	T value;
	auto const r = std::from_chars(beg, end, value, radix);

	if (r.ec != std::errc{})
	{
		return vsm::unexpected(r.ec);
	}

	if (r.ptr != end)
	{
		return vsm::unexpected(std::errc::invalid_argument);
	}

	return value;
}

template<std::floating_point T>
vsm::result<T> from_chars(std::string_view const string, std::chars_format const format = std::chars_format::general)
{
	char const* const beg = string.data();
	char const* const end = string.data() + string.size();

	T value;
	auto const r = std::from_chars(beg, end, value, format);

	if (r.ec != std::errc{})
	{
		return vsm::unexpected(r.ec);
	}

	if (r.ptr != end)
	{
		return vsm::unexpected(std::errc::invalid_argument);
	}

	return value;
}

} // namespace vsm
