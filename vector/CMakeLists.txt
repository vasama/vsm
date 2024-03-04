cmake_minimum_required(VERSION 3.23)

project(vsm_vector)

vsm_find_sibling(vsm-algorithm)
vsm_find_sibling(vsm-allocator)
vsm_find_sibling(vsm-core)

vsm_add_library(vsm::vector
	HEADERS
		include/vsm/vector.hpp

	HEADER_DEPENDENCIES
		vsm::allocator
		vsm::algorithm
		vsm::core

	TEST_SOURCES
		source/vsm/test/vector.cpp
)
