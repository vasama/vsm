#pragma once

#include <system_error>

namespace vsm::cli {
namespace detail {

struct error_category : std::error_category
{
	virtual char const* name() const noexcept override;
	virtual std::string message(int code) const override;

	static error_category const instance;
};

} // namespace detail

enum class error
{
	success = 0,

	invalid_syntax,
	unrecognized_option,
	invalid_value,
	command_not_given,
	option_given_too_few_times,
	option_given_too_many_times,
};

[[nodiscard]] inline std::error_code make_error_code(error const e)
{
	return std::error_code(static_cast<int>(e), detail::error_category::instance);
}

} // namespace vsm::cli

template<>
struct std::is_error_code_enum<vsm::cli::error>
{
	static constexpr bool value = true;
};
