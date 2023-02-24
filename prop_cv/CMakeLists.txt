cmake_minimum_required(VERSION 3.24)
project(vsm_prop_cv)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::prop_cv

	HEADERS
		include/vsm/prop_cv.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/prop_cv.cpp

	TEST_DEPENDENCIES
		vsm::test_core
)
