cmake_minimum_required(VERSION 3.24)

project(vsm_unique_resource)

vsm_find_sibling(vsm-core)

vsm_add_library(vsm::unique_resource
	HEADERS
		include/vsm/unique_resource.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/unique_resource.cpp
)
