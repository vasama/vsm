cmake_minimum_required(VERSION 3.24)
project(vsm_unique_resource)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::unique_resource

	HEADERS
		include/vsm/out_resource.hpp
		include/vsm/unique_resource.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/unique_resource.cpp
)
