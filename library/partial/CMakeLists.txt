cmake_minimum_required(VERSION 3.24)
project(vsm_partial)

find_package(vsm.cmake REQUIRED)
vsm_define_package(vsm.partial)

vsm_add_library(
	vsm::partial

	HEADERS
		include/vsm/partial.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/partial.cpp

	TEST_DEPENDENCIES
		vsm::testing::core

	TEST_COMPILE_ONLY
)
