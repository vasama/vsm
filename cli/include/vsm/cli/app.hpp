#pragma once

#include <vsm/cli/base.hpp>

#include <vsm/function.hpp>
#include <vsm/partial.hpp>
#include <vsm/result.hpp>

#include <span>
#include <string_view>

namespace vsm::cli {

class app : detail::base
{
	std::string_view m_help;
	std::string_view m_help_header;
	std::string_view m_help_footer;
	std::string_view m_help_usage;

public:
	using handler_type = function<void()>;
	using error_reporter_type = function<void(std::string_view)>;


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

	group& group(std::string_view name);

	option& option(std::string_view name);
	option& flag(std::string_view name);


	result<void> parse(std::span<std::string_view const> args);
	result<void> parse(std::span<char const* const> args);

private:
	vsm_partial(app);
};

std::unique_ptr<app> make_app(std::string_view name);

} // namespace vsm::cli
