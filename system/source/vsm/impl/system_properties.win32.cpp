#include <vsm/system_properties.hpp>

#include <Windows.h>

using namespace vsm;

static system_properties query_system_properties()
{
	SYSTEM_INFO system_info;
	GetSystemInfo(&system_info);

	return
	{
		.logical_processor_count = 
	};
}

system_properties const& vsm::get_system_properties()
{
	system_properties const result = query_system_properties();
	return result;
}
