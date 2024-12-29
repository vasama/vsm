cmake_minimum_required(VERSION 3.24)
project(vsm_shared_string)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

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
		vsm::testing::allocator
		vsm::testing::core
)
