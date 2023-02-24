#pragma once

#include <vsm/cli/app.hpp>

namespace vsm::cli {

using app_internal = partial::internal_type<app>;

class app::internal_class : public app
{
public:
	uint32_t m_visible_command_count = 0;
	uint32_t m_visible_positional_count = 0;
	uint32_t m_visible_group_count = 0;
	uint32_t m_visible_option_count = 0;
	bool m_has_required_options = false;


	group* create_group(resource* resource, std::string_view name);
	option* create_option(resource* resource, std::string_view name, partial::internal_class<group>* group, bool flag);
};

} // namespace vsm::cli
