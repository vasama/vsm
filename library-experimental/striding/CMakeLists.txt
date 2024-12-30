cmake_minimum_required(VERSION 3.24)
project(vsm_striding)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::striding

	HEADERS
		include/vsm/striding_ptr.hpp
		include/vsm/striding_span.hpp

	HEADER_DEPENDENCIES
		vsm::core
)
