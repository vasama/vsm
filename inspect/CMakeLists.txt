cmake_minimum_required(VERSION 3.23)

project(vsm_inspect)

vsm_add_library(vsm_inspect
	EXPORT_ALIAS vsm::inspect

	HEADERS
		include/vsm/inspect.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/inspect.cpp
)
