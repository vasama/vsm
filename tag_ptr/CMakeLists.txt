cmake_minimum_required(VERSION 3.24)
project(vsm_tag_ptr)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::tag_ptr

	HEADERS
		include/vsm/tag_ptr.hpp

	HEADER_DEPENDENCIES
		vsm::core

	VISUALIZERS
		visualizers/vsm/tag_ptr.natvis

	TEST_SOURCES
		source/vsm/test/tag_ptr.cpp

	TEST_DEPENDENCIES
		vsm::test_core
)
