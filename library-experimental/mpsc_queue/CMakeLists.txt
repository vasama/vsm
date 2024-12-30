cmake_minimum_required(VERSION 3.24)
project(vsm_mpsc_queue)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::mpsc_queue

	HEADERS
		include/vsm/bounded_mpsc_queue.hpp
		include/vsm/mpsc_queue.hpp

	TEST_SOURCES
		source/vsm/test/bounded_mpsc_queue.cpp
		source/vsm/test/mpsc_queue.cpp

	HEADER_DEPENDENCIES
		vsm::allocator
		vsm::core
		vsm::math
		vsm::intrusive_ptr
)
