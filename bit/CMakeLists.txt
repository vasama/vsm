cmake_minimum_required(VERSION 3.24)
project(vsm_bit)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::bit

	HEADERS
		include/vsm/bit_array.hpp
		include/vsm/bit_ptr.hpp
		include/vsm/bit_span.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/bit_ptr.cpp

	TEST_DEPENDENCIES
		vsm::test_core
)
