cmake_minimum_required(VERSION 3.24)
project(vsm_core)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::numeric

	HEADERS
		include/vsm/numeric.hpp

	HEADER_DEPENDENCIES
		vsm::core
		vsm::result
)
