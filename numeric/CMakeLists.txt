cmake_minimum_required(VERSION 3.24)

project(vsm_core)

vsm_find_sibling(vsm-core)

vsm_add_library(vsm::numeric
	HEADERS
		include/vsm/integer_conversion.hpp

	HEADER_DEPENDENCIES
		vsm::core
		vsm::result
)
