cmake_minimum_required(VERSION 3.24)
project(vsm_any)

find_package(vsm.cmake REQUIRED)
vsm_define_package(vsm.any)

vsm_add_library(
	vsm::any

	HEADERS
		include/vsm/any_ref.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/any_ref.cpp
)
