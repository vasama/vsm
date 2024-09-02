cmake_minimum_required(VERSION 3.24)

project(vsm_vector)

find_package(vsm-cmake REQUIRED)

vsm_find_sibling(vsm-algorithm)
vsm_find_sibling(vsm-allocator)
vsm_find_sibling(vsm-core)
vsm_find_sibling(vsm-test_tools)

vsm_add_library(vsm::vector
	HEADERS
		include/vsm/vector.hpp

	HEADER_DEPENDENCIES
		vsm::allocator
		vsm::algorithm
		vsm::core

	TEST_SOURCES
		source/vsm/test/vector.cpp

	TEST_DEPENDENCIES
		vsm::test_tools
)

vsm_configure(
	vsm::vector
	PLATFORM Windows

	VISUALIZERS
		visualizers/vsm/vector.natvis
)
