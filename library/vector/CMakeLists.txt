cmake_minimum_required(VERSION 3.24)
project(vsm_vector)

find_package(vsm.cmake REQUIRED)
vsm_define_package(vsm.vector)

vsm_add_library(
	vsm::vector

	HEADERS
		include/vsm/vector.hpp

	VISUALIZERS
		visualizers/vsm/vector.natvis

	HEADER_DEPENDENCIES
		vsm::allocator
		vsm::algorithm
		vsm::core

	TEST_SOURCES
		source/vsm/test/vector.cpp

	TEST_DEPENDENCIES
		vsm::testing::allocator
		vsm::testing::core
)
