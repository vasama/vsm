#pragma once

#include <vsm/concepts.hpp>
#include <vsm/from_chars.hpp>
#include <vsm/result.hpp>
#include <vsm/tag_invoke.hpp>

#include <string_view>

namespace vsm::cli {

struct parse_cpo
{
	friend std::string_view tag_invoke(parse_cpo, tag<std::string_view>, std::string_view const value)
	{
		return string;
	}

	friend result<char> tag_invoke(parse_cpo, tag<char>, std::string_view const string)
	{

	}

	friend result<bool> tag_invoke(parse_cpo, tag<bool>, std::string_view const string)
	{

	}

	template<arithmetic T>
	friend result<T> tag_invoke(parse_cpo, tag<T>, std::string_view const string)
	{
		return from_chars<T>(string);
	}

	template<enumeration T>
	friend result<T> tag_invoke(parse_cpo, tag<T>, std::string_view const string);

	template<non_cvref T>
	decltype(auto) operator()(tag<T> const tag, std::string_view const string) const
		requires tag_invocable<parse_cpo, tag<T>, std::string_view>
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
