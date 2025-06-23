cmake_minimum_required(VERSION 3.24)
project(vsm_base64)

find_package(vsm.cmake REQUIRED)
vsm_define_package(vsm.base64)

vsm_add_library(
	vsm::base64

	HEADERS
		include/vsm/base64.hpp
		include/vsm/base64_streaming.hpp

	SOURCES
		source/vsm/impl/base64_streaming.cpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/base64_streaming.cpp
)
