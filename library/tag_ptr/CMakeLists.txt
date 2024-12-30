cmake_minimum_required(VERSION 3.24)
project(vsm_tag_ptr)

find_package(vsm.cmake REQUIRED)
vsm_define_package(vsm.tag_ptr)

vsm_add_library(
	vsm::tag_ptr

	HEADERS
		include/vsm/tag_ptr.hpp

	VISUALIZERS
		visualizers/vsm/tag_ptr.natvis

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/tag_ptr.cpp

	TEST_DEPENDENCIES
		vsm::testing::core
)
