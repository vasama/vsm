cmake_minimum_required(VERSION 3.23)

project(vsm_test_tools)

vsm_add_library(vsm::test_tools
	HEADERS
		include/vsm/test/dependent_context.hpp
		include/vsm/test/instance_counter.hpp

	TEST_SOURCES
		source/vsm/test/dependent_context.cpp
)
