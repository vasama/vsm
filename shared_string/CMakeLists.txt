cmake_minimum_required(VERSION 3.23)

project(vsm_shared_string)

vsm_find_sibling(vsm-core)
vsm_find_sibling(vsm-allocator)
vsm_find_sibling(vsm-vector)

vsm_add_library(vsm::shared_string
	HEADERS
		include/vsm/shared_string.hpp

	HEADER_DEPENDENCIES
		vsm::allocator
		vsm::core
		vsm::vector

	TEST_SOURCES
		source/vsm/test/shared_string.cpp
)
