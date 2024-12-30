cmake_minimum_required(VERSION 3.24)
project(vsm_intrusive_ptr)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::intrusive_ptr

	HEADERS
		include/vsm/atomic_intrusive_ptr.hpp
		include/vsm/intrusive_ptr.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/atomic_intrusive_ptr.cpp
		source/vsm/test/intrusive_ptr.cpp

	TEST_DEPENDENCIES
		vsm::testing::core
)
