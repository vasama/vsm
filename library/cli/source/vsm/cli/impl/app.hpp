#pragma once

#include <vsm/cli/app.hpp>

#include <vsm/cli/impl/group.hpp>

#include <format>

namespace vsm::cli {

using app_internal = partial::internal_class<app>;
class app::internal_class : public app
{
public:
	uint32_t m_visible_command_count = 0;
	uint32_t m_visible_positional_count = 0;
	uint32_t m_visible_group_count = 0;
	uint32_t m_visible_option_count = 0;
	bool m_has_required_options = false;


	group_internal& create_group(resource& resource, std::string_view name);

	option_internal& create_option(
		resource& resource,
		std::string_view spec,
		group_internal* group,
		bool flag);

	template<typename... Args>
	void report_configuration_error(std::format_string<Args...> const format, Args&&... args) const
	{
		_report_configuration_error(format.get(), std::make_format_args(args...));
	}

	template<typename... Args>
	void report_error(std::format_string<Args...> const format, Args&&... args) const
	{
		_report_error(format.get(), std::make_format_args(args...));
	}

private:
	void _report_configuration_error(std::string_view format, std::format_args args) const;
	void _report_error(std::string_view format, std::format_args args) const;
};

} // namespace vsm::cli
