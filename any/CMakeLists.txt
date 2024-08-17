cmake_minimum_required(VERSION 3.24)

project(vsm_any)

vsm_find_sibling(vsm-core)

vsm_add_library(
	vsm::any

	HEADERS
		include/vsm/any_ref.hpp

		include/vsm/any_ref.hpp
		include/vsm/any.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/any_ref.cpp
		source/vsm/test/any.cpp
)
