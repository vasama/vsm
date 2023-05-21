cmake_minimum_required(VERSION 3.23)

project(vsm_propagate_const)

vsm_find_sibling(vsm-core)
vsm_find_sibling(vsm-test_tools)

vsm_add_library(vsm_propagate_const
	EXPORT_ALIAS vsm::propagate_const

	HEADERS
		include/vsm/propagate_const.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/propagate_const.cpp
		
	TEST_DEPENDENCIES
		vsm::test_tools
)
