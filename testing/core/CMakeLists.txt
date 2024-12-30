cmake_minimum_required(VERSION 3.24)
project(vsm_testing_core)

find_package(vsm.cmake REQUIRED)
vsm_define_package(vsm.testing.core)

vsm_add_library(
	vsm::testing::core

	HEADERS
		include/vsm/testing/dependent_context.hpp
		include/vsm/testing/instance_counter.hpp

	SOURCES
		source/vsm/impl/instance_counter.cpp

	HEADER_LINK_LIBRARIES
		Catch2::Catch2

	TEST_FRAMEWORK_LIBRARY
		Catch2::Catch2WithMain

	TEST_SOURCES
		source/vsm/test/dependent_context.cpp
		source/vsm/test/instance_counter.cpp
)
