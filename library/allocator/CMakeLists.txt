cmake_minimum_required(VERSION 3.24)
project(vsm_allocator)

find_package(vsm.cmake REQUIRED)
vsm_define_package(vsm.allocator)

vsm_add_library(vsm::allocator
	HEADERS
		include/vsm/any_allocator.hpp
		include/vsm/any_monotonic_allocator.hpp
		include/vsm/block_pool_resource.hpp

	HEADER_LINK_LIBRARIES
		vsm::any
		vsm::core

	TEST_SOURCES
		source/vsm/test/any_allocator.cpp
		source/vsm/test/any_monotonic_allocator.cpp

	TEST_LINK_LIBRARIES
		vsm::testing::allocator
)
