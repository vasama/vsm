cmake_minimum_required(VERSION 3.24)
project(vsm_hash)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::hash

	HEADERS
		include/vsm/default_hash.hpp

	HEADER_DEPENDENCIES
		vsm::core
		xxHash::xxhash
)
