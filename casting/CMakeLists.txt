cmake_minimum_required(VERSION 3.24)

project(vsm_container_core)

find_package(vsm-cmake REQUIRED)

vsm_find_sibling(vsm-core)

vsm_add_library(
	vsm::casting

	HEADERS
		include/vsm/casting.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/casting.cpp
)
