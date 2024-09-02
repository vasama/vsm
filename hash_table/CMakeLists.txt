cmake_minimum_required(VERSION 3.24)

project(vsm_hash_table)

find_package(vsm-cmake REQUIRED)

vsm_find_sibling(vsm-allocator)
vsm_find_sibling(vsm-container_core)
vsm_find_sibling(vsm-core)
vsm_find_sibling(vsm-hash)
vsm_find_sibling(vsm-math)
vsm_find_sibling(vsm-test_tools)

vsm_add_library(
	vsm::hash_table

	HEADERS
		include/vsm/detail/deterministic_table.hpp
		include/vsm/detail/swiss_table.hpp
		include/vsm/detail/swiss_table.ipp

		include/vsm/hash_map.hpp
		include/vsm/hash_set.hpp

		include/vsm/deterministic_map.hpp
		include/vsm/deterministic_set.hpp
		include/vsm/swiss_map.hpp
		include/vsm/swiss_set.hpp

	SOURCES
		source/vsm/impl/deterministic_table.cpp
		source/vsm/impl/swiss_table.cpp

	HEADER_DEPENDENCIES
		vsm::allocator
		vsm::container_core
		vsm::core
		vsm::hash
		vsm::math

	TEST_SOURCES
		source/vsm/test/deterministic_set.cpp
		source/vsm/test/deterministic_map.cpp
		source/vsm/test/swiss_set.cpp
		source/vsm/test/swiss_map.cpp

	TEST_DEPENDENCIES
		vsm::test_tools
)
