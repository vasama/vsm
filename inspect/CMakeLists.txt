cmake_minimum_required(VERSION 3.24)
project(vsm_inspect)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::inspect

	HEADERS
		include/vsm/inspect.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/inspect.cpp
)
