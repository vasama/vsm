cmake_minimum_required(VERSION 3.24)
project(vsm_test_tools)

find_package(vsm-cmake REQUIRED)

vsm_add_library(
	vsm::test_core

	HEADERS
		include/vsm/test/dependent_context.hpp
		include/vsm/test/instance_counter.hpp

	SOURCES
		source/vsm/impl/instance_counter.cpp

	TEST_SOURCES
		source/vsm/test/dependent_context.cpp
)
