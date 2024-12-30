cmake_minimum_required(VERSION 3.24)
project(vsm_optional)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::optional

	HEADERS
		include/vsm/optional.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/optional.cpp
)
