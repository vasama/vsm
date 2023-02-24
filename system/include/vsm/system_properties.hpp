#pragma once

#include <cstddef>

namespace vsm {

struct system_properties
{
	size_t logical_processor_count;
	size_t physical_processor_count;
};

system_properties const& get_system_properties();

} // namespace vsm
