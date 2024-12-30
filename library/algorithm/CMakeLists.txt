cmake_minimum_required(VERSION 3.24)

project(vsm_algorithm)

find_package(vsm.cmake REQUIRED)
vsm_define_package(vsm.algorithm)

vsm_add_library(
	vsm::algorithm

	HEADERS
		include/vsm/algorithm/exponential_lower_bound.hpp
		include/vsm/algorithm/remove_unstable.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/algorithm/test/exponential_lower_bound.cpp
		source/vsm/algorithm/test/remove_unstable.cpp
)
