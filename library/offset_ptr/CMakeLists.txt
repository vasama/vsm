cmake_minimum_required(VERSION 3.24)
project(vsm_offset_ptr)

find_package(vsm.cmake REQUIRED)
vsm_define_package(vsm.offset_ptr)

vsm_add_library(
	vsm::offset_ptr

	HEADERS
		include/vsm/offset_ptr.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/offset_ptr.cpp
)
