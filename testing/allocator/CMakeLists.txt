cmake_minimum_required(VERSION 3.24)
project(vsm_test_allocator)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::testing::allocator

	HEADERS
		include/vsm/testing/allocator.hpp

	SOURCES
		source/vsm/impl/allocator.hpp
		source/vsm/impl/allocator.cpp

	HEADER_DEPENDENCIES
		vsm::core
)

vsm_configure(
	vsm::testing::allocator

	PLATFORM Windows

	SOURCES
		source/vsm/impl/windows/allocator.cpp
)

vsm_configure(
	vsm::testing::allocator

	PLATFORM Linux

	SOURCES
		source/vsm/impl/posix/allocator.cpp
)
