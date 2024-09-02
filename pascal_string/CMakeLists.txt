cmake_minimum_required(VERSION 3.24)

project(vsm_pascal_string)

find_package(vsm-cmake REQUIRED)

vsm_find_sibling(vsm-core)

vsm_add_library(
	vsm::pascal_string

	HEADERS
		include/vsm/pascal_string.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/pascal_string.cpp
)
