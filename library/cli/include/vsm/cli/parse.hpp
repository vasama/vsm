#pragma once

#include <vsm/cli/error.hpp>

#include <vsm/concepts.hpp>
#include <vsm/result.hpp>
#include <vsm/tag_invoke.hpp>

#include <charconv>
#include <string_view>

namespace vsm::cli {

struct parse_cpo
{
	friend std::string_view tag_invoke(parse_cpo, tag<std::string_view>, std::string_view const value)
	{
		return value;
	}

	friend result<char> tag_invoke(parse_cpo, tag<char>, std::string_view const string)
	{
		//TODO: Implement parse_cpo for char
		return vsm::unexpected(error::invalid_value);
	}

	friend result<bool> tag_invoke(parse_cpo, tag<bool>, std::string_view const string)
	{
		if (string == "true")
		{
			return true;
		}
		
		if (string == "false")
		{
			return false;
		}

		return vsm::unexpected(error::invalid_value);
	}

	template<arithmetic T>
	friend result<T> tag_invoke(parse_cpo, tag<T>, std::string_view const string)
	{
		T value;
		auto const [ec, pos] = std::from_chars(
			string.data(),
			string.data() + string.size(),
			value);

		if (ec)
		{
			return vsm::unexpected(ec);
		}

		return value;
	}

	template<enumeration T>
	friend result<T> tag_invoke(parse_cpo, tag<T>, std::string_view const string);

	template<non_cvref T>
	decltype(auto) operator()(tag<T> const tag, std::string_view const string) const
		requires tag_invocable<parse_cpo, vsm::tag<T>, std::string_view>
	{
		return tag_invoke(*this, tag, string);
	}
};

template<non_cvref T>
result<T> parse(std::string_view const string)
{
	return parse_cpo()(tag<T>(), string);
}

} // namespace vsm::cli
