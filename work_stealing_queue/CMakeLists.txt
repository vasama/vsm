cmake_minimum_required(VERSION 3.23)

project(vsm_work_stealing_queue)

vsm_find_sibling(vsm-core)
vsm_find_sibling(vsm-intrusive_ptr)

vsm_add_library(vsm::work_stealing_queue
	HEADERS
		include/vsm/bounded_work_stealing_queue.hpp
		include/vsm/work_stealing_queue.hpp

	HEADER_DEPENDENCIES
		vsm::core
		vsm::intrusive_ptr

	TEST_SOURCES
		source/vsm/impl/work_stealing_queue.test.cpp
)
