cmake_minimum_required(VERSION 3.23)

project(vsm_intrusive_ptr)

vsm_find_sibling(vsm-core)
vsm_find_sibling(vsm-test_tools)

vsm_add_library(vsm::intrusive_ptr
	HEADERS
		include/vsm/atomic_intrusive_ptr.hpp
		include/vsm/intrusive_ptr.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/atomic_intrusive_ptr.cpp
		source/vsm/test/intrusive_ptr.cpp

	TEST_DEPENDENCIES
		vsm::test_tools
)
