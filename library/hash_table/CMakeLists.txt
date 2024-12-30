cmake_minimum_required(VERSION 3.24)
project(vsm_hash_table)

find_package(vsm.cmake REQUIRED)
vsm_define_package(vsm.hash_table)

vsm_add_library(
	vsm::hash_table

	HEADERS
		include/vsm/hash_map.hpp
		include/vsm/hash_set.hpp

		include/vsm/array_map.hpp
		include/vsm/array_set.hpp

		include/vsm/swiss_map.hpp
		include/vsm/swiss_set.hpp

		include/vsm/detail/array_table.hpp
		include/vsm/detail/hash_table.hpp
		include/vsm/detail/swiss_table.hpp

	SOURCES
		source/vsm/impl/array_table.cpp
		source/vsm/impl/swiss_table.cpp

	VISUALIZERS
		visualizers/vsm/hash_table.natvis

	HEADER_LINK_LIBRARIES
		vsm::container_core
		vsm::core
		vsm::hash

	TEST_SOURCES
		source/vsm/test/array_map.cpp
		source/vsm/test/array_set.cpp
		source/vsm/test/swiss_map.cpp
		source/vsm/test/swiss_set.cpp
		source/vsm/test/swiss_table.cpp

	TEST_LINK_LIBRARIES
		vsm::testing::allocator
)
