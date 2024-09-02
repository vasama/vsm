cmake_minimum_required(VERSION 3.24)

project(vsm_allocator)

find_package(vsm-cmake REQUIRED)

vsm_find_sibling(vsm-test_tools)

vsm_add_library(
	vsm::allocator_core

	HEADERS
		include/vsm/allocator.hpp
		include/vsm/default_allocator.hpp
		include/vsm/new_allocator.hpp

	HEADER_DEPENDENCIES
		vsm::core
)

if(DEFINED vsm_aggregate_test_target)
	vsm_find_sibling(vsm-test_tools)

	vsm_configure(
		vsm::allocator_core

		TEST_SOURCES
			source/vsm/test/allocator.cpp
	
		TEST_DEPENDENCIES
			vsm::test_tools
	)
endif()
