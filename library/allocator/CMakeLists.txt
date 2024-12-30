cmake_minimum_required(VERSION 3.24)
project(vsm_allocator)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

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
		source/vsm/test/any_allocator.cpp

	TEST_DEPENDENCIES
		vsm::testing::allocator
)
