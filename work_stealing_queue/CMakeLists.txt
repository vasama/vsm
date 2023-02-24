cmake_minimum_required(VERSION 3.24)
project(vsm_work_stealing_queue)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::work_stealing_queue

	HEADERS
		include/vsm/bounded_work_stealing_queue.hpp
		include/vsm/work_stealing_queue.hpp

	HEADER_DEPENDENCIES
		vsm::allocator
		vsm::core
		vsm::intrusive_ptr
		vsm::math

	TEST_SOURCES
		source/vsm/test/bounded_work_stealing_queue.cpp
		source/vsm/test/work_stealing_queue.cpp
)
