cmake_minimum_required(VERSION 3.24)
project(vsm_test_allocator)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::test_allocator

	HEADERS
		include/vsm/test/allocator.hpp

	SOURCES
		source/vsm/impl/allocator.hpp
		source/vsm/impl/allocator.cpp

	HEADER_DEPENDENCIES
		vsm::core
)

vsm_configure(
	vsm::test_allocator

	PLATFORM Windows

	SOURCES
		source/vsm/impl/windows/allocator.cpp
)
