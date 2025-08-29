#pragma once

#include <vsm/cli/detail/base.hpp>
#include <vsm/cli/group.hpp>

#include <vsm/function.hpp>
#include <vsm/partial.hpp>
#include <vsm/result.hpp>
#include <vsm/streams/any_ref.hpp>

#include <span>
#include <string_view>

namespace vsm::cli {

class app : protected detail::base
{
	std::string_view m_help;
	std::string_view m_help_header;
	std::string_view m_help_footer;
	std::string_view m_help_usage;

public:
	using handler_type = function<void() const>;
	using error_reporter_type = function<void(std::string_view) const>;


	app& hide();

	app& help(std::string_view const text)
	{
		m_help = text;
		return *this;
	}

	app& help_header(std::string_view const text)
	{
		m_help_header = text;
		return *this;
	}

	app& help_footer(std::string_view const text)
	{
		m_help_footer = text;
		return *this;
	}

	app& usage(std::string_view const text)
	{
		m_help_usage = text;
		m_flags = set_flags(m_flags, flags::show_usage, !text.empty());
		return *this;
	}


	app& require_command()
	{
		m_flags |= flags::require_command;
		return *this;
	}


	app& inclusive_lock(resource& resource);
	app& exclusive_lock(resource& resource);


	app& command(std::string_view name);

	cli::group& group(std::string_view name);

	cli::option& option(std::string_view name);
	cli::option& flag(std::string_view name);


	result<void> parse(std::span<std::string_view const> args);
	result<void> parse(std::span<char const* const> args);

	result<void> print_help_to_stdout() const;
	result<void> print_help_to_stderr() const;
	result<void> print_help(streams::any_sink_ref sink) const;

	void set_error_reporter(error_reporter_type error_reporter);

private:
	vsm_partial(app);
	vsm_partial_delete(app);
};
using app_ptr = std::unique_ptr<app>;

app_ptr make_app(std::string_view name);

} // namespace vsm::cli
