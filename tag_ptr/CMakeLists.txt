cmake_minimum_required(VERSION 3.23.0)

project(vsm_tag_ptr)

vsm_find_sibling(vsm-core)
vsm_find_sibling(vsm-test_tools)

vsm_add_library(vsm_tag_ptr
	EXPORT_ALIAS vsm::tag_ptr

	HEADERS
		include/vsm/tag_ptr.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/tag_ptr.cpp

	TEST_DEPENDENCIES
		vsm::test_tools
)
