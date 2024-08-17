cmake_minimum_required(VERSION 3.24)

project(vsm_hash)

vsm_find_sibling(vsm-core)

vsm_add_library(
	vsm::hash

	HEADERS
		include/vsm/default_hash.hpp

	SOURCES
		source/vsm/impl/default_hash.cpp

	HEADER_DEPENDENCIES
		vsm::core
)
