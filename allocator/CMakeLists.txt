cmake_minimum_required(VERSION 3.24)

project(vsm_allocator)

vsm_find_sibling(vsm-any)
vsm_find_sibling(vsm-test_tools)

vsm_add_library(vsm::allocator
	HEADERS
		include/vsm/allocator.hpp
		include/vsm/any_allocator.hpp
		include/vsm/default_allocator.hpp
		include/vsm/new_allocator.hpp

	HEADER_DEPENDENCIES
		vsm::any
		vsm::core

	TEST_SOURCES
		source/vsm/test/allocator.cpp
		source/vsm/test/any_allocator.cpp

	TEST_DEPENDENCIES
		vsm::test_tools
)
