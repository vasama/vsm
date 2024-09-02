cmake_minimum_required(VERSION 3.24)

project(vsm_test_tools)

find_package(vsm-cmake REQUIRED)

vsm_find_sibling(vsm-allocator_core)
vsm_find_sibling(vsm-core)

vsm_add_library(
	vsm::test_tools

	HEADERS
		include/vsm/test/dependent_context.hpp
		include/vsm/test/instance_counter.hpp

	SOURCES
		source/vsm/impl/allocator.hpp
		source/vsm/impl/allocator.cpp
		source/vsm/impl/instance_counter.cpp

	HEADER_DEPENDENCIES
		vsm::allocator_core
		vsm::core

	TEST_SOURCES
		source/vsm/test/dependent_context.cpp
)

vsm_configure(
	vsm::test_tools

	PLATFORM Windows

	SOURCES
		source/vsm/impl/windows/allocator.cpp
)
