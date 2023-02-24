cmake_minimum_required(VERSION 3.24)
project(vsm_result)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::result

	HEADERS
		include/vsm/result.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/result.cpp

	TEST_DEPENDENCIES
		vsm::test_core
)
