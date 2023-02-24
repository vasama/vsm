#pragma once

#include <vsm/error_code.hpp>

namespace vsm::cli {
namespace detail {

struct error_category : vsm::error_category
{
	virtual char const* name() const noexcept override;
	virtual std::string_view format(int code, std::span<char> buffer) const noexcept override;
};

} // namespace detail

enum class error
{
	success = 0,

	invalid_syntax,
	invalid_value,
	command_not_given,
	option_given_too_few_times,
	option_given_too_many_times,
};

} // namespace vsm::cli
