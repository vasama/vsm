cmake_minimum_required(VERSION 3.24)

project(vsm_shared_string)

vsm_find_sibling(vsm-core)
vsm_find_sibling(vsm-allocator)
vsm_find_sibling(vsm-test_tools)
vsm_find_sibling(vsm-vector)

vsm_add_library(
	vsm::shared_vector

	HEADERS
		include/vsm/shared_vector.hpp

	HEADER_DEPENDENCIES
		vsm::allocator
		vsm::core
		vsm::vector

	TEST_SOURCES
		source/vsm/test/shared_vector.cpp

	TEST_DEPENDENCIES
		vsm::test_tools
)
