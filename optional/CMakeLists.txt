cmake_minimum_required(VERSION 3.23)

project(vsm_optional)

vsm_find_sibling(vsm-core)

vsm_add_library(vsm::optional
	HEADERS
		include/vsm/optional.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/optional.cpp
)
