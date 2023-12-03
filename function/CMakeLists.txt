cmake_minimum_required(VERSION 3.23)

project(vsm_function)

vsm_find_sibling(vsm-core)

vsm_add_library(vsm::function
	HEADERS
		include/vsm/function_view.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/function_view.cpp
)
