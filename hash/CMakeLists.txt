cmake_minimum_required(VERSION 3.23)

project(vsm_hash)

vsm_add_library(vsm::hash
	HEADERS
		include/vsm/default_hash.hpp
		include/vsm/hash.hpp

	SOURCES
		source/vsm/impl/default_hash.cpp

	HEADER_DEPENDENCIES
		vsm::core
)
