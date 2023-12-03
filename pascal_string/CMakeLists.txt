cmake_minimum_required(VERSION 3.23)

project(vsm_pascal_string)

vsm_find_sibling(vsm-core)

vsm_add_library(vsm::pascal_string
	HEADERS
		include/vsm/pascal_string.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/pascal_string.cpp
)
