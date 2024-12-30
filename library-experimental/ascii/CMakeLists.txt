cmake_minimum_required(VERSION 3.24)
project(vsm_ascii)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::ascii

	HEADERS
		include/vsm/ascii.hpp

	SOURCES
		source/vsm/impl/ascii.cpp

	SOURCE_DEPENDENCIES
		vsm::core
)
