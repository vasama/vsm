cmake_minimum_required(VERSION 3.24)

project(vsm_algorithm)

find_package(vsm-cmake REQUIRED)

vsm_find_sibling(vsm-core)

vsm_add_library(
	vsm::algorithm

	HEADERS
		include/vsm/algorithm/remove_unstable.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/algorithm/test/remove_unstable.cpp
)
