cmake_minimum_required(VERSION 3.24)

project(vsm_math)

find_package(vsm-cmake REQUIRED)

vsm_add_library(
	vsm::math

	HEADERS
		include/vsm/math.hpp

	TEST_SOURCES
		source/vsm/test/math.cpp
)
