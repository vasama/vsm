cmake_minimum_required(VERSION 3.24)
project(vsm_test_tools)

find_package(vsm-cmake REQUIRED)

vsm_add_library(
	vsm::testing::core

	HEADERS
		include/vsm/testing/dependent_context.hpp
		include/vsm/testing/instance_counter.hpp

	SOURCES
		source/vsm/impl/instance_counter.cpp

	TEST_SOURCES
		source/vsm/test/dependent_context.cpp
		source/vsm/test/instance_counter.cpp
)
