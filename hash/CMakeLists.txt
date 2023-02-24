cmake_minimum_required(VERSION 3.24)
project(vsm_hash)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::hash

	HEADERS
		include/vsm/default_hash.hpp

	SOURCES
		source/vsm/impl/default_hash.cpp

	HEADER_DEPENDENCIES
		vsm::core
)
