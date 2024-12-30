#pragma once

namespace vsm::cli {

using option_internal = partial::internal_type<option>;

class option::internal_class : public option
{
public:
	std::string_view m_help_option;
	std::string_view m_help_positional;

	size_t m_count = 0;

	static std::unique_ptr<internal_class> create(
		partial::internal_class<app>* app, resource& parent,
		partial::internal_type<group>* group, bool flag);


	result<void> process_argument(std::string_view form, std::string_view value);
	result<void> process_completed();
};

} // namespace vsm::cli
