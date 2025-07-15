#pragma once

#include <vsm/cli/group.hpp>

#include <vsm/cli/app.hpp>
#include <vsm/cli/impl/option.hpp>

namespace vsm::cli {

using group_internal = partial::internal_class<group>;
class group::internal_class : public group
{
public:
	static std::unique_ptr<internal_class> create(
		partial::internal_class<app>* app,
		resource& parent,
		std::string_view name);

	result<void> process_argument();
	result<void> process_completed();
};

} // namespace vsm::cli
