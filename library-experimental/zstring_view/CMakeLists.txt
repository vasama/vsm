cmake_minimum_required(VERSION 3.24)
project(vsm_zstring_view)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::zstring_view

	HEADERS
		include/vsm/zstring_view.hpp

	HEADER_DEPENDENCIES
		vsm::core
)
