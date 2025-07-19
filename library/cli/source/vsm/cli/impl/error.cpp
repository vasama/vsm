#include <vsm/cli/error.hpp>

#include <format>

using namespace vsm;
using namespace vsm::cli;

char const* cli::detail::error_category::name() const noexcept
{
	return "vsm::cli";
}

std::string cli::detail::error_category::message(int const code) const
{
	switch (static_cast<cli::error>(code))
	{
	case cli::error::success:
		return "The operation completed successfully.";

	case cli::error::invalid_syntax:
		return "invalid syntax";

	case cli::error::unrecognized_option:
		return "An unrecognized option was specified.";

	case cli::error::invalid_value:
		return "An invalid value was given for an option.";

	case cli::error::command_not_given:
		return "A command is required but was not specified.";

	case cli::error::option_given_too_few_times:
		return "An option was specified fewer times than is required.";

	case cli::error::option_given_too_many_times:
		return "An option was specified more times than is permitted.";
	}

	return std::format("vsm::cli:{}", code);
}

cli::detail::error_category const cli::detail::error_category::instance;
