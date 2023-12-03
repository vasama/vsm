cmake_minimum_required(VERSION 3.23)

project(vsm_hash_table)

vsm_find_sibling(vsm-allocator)
vsm_find_sibling(vsm-container_core)
vsm_find_sibling(vsm-core)
vsm_find_sibling(vsm-hash)
vsm_find_sibling(vsm-math)

vsm_add_library(vsm::hash_table
	HEADERS
		include/vsm/detail/swiss_table.hpp
		include/vsm/hash_map.hpp
		include/vsm/hash_set.hpp

	SOURCES
		source/vsm/impl/swiss_table.cpp

	HEADER_DEPENDENCIES
		vsm::allocator
		vsm::container_core
		vsm::core
		vsm::hash
		vsm::math

	TEST_SOURCES
		source/vsm/test/hash_map.cpp
		source/vsm/test/hash_set.cpp
)
