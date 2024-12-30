cmake_minimum_required(VERSION 3.24)
project(vsm_hash)

find_package(vsm.cmake REQUIRED)
vsm_define_package(vsm.hash)

vsm_add_library(
	vsm::hash

	HEADERS
		include/vsm/default_hash.hpp
		include/vsm/xxhash.hpp

	HEADER_DEPENDENCIES
		vsm::core
		xxHash::xxhash

	TEST_SOURCES
		source/vsm/test/policy.cpp
		source/vsm/test/xxhash.cpp
)
