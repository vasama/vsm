cmake_minimum_required(VERSION 3.23)

project(vsm_allocator)

vsm_add_library(vsm::allocator
	HEADERS
		include/vsm/allocator.hpp
		include/vsm/default_allocator.hpp
		include/vsm/new_allocator.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/allocator.cpp
)
