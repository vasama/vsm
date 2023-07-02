cmake_minimum_required(VERSION 3.23)

project(vsm_result)

vsm_find_sibling(vsm-core)
vsm_find_sibling(vsm-test_tools)

vsm_add_library(vsm_result
	EXPORT_ALIAS vsm::result

	HEADERS
		include/vsm/result.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/result.cpp

	TEST_DEPENDENCIES
		vsm::test_tools
)
