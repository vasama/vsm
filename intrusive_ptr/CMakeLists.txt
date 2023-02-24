cmake_minimum_required(VERSION 3.23.0)

project(vsm_intrusive_ptr)

vsm_find_sibling(vsm-core)

vsm_add_library(vsm_intrusive_ptr
	EXPORT_ALIAS vsm::intrusive_ptr

	HEADERS
		include/vsm/atomic_intrusive_ptr.hpp
		include/vsm/intrusive_ptr.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/impl/atomic_intrusive_ptr.test.cpp
		source/vsm/impl/intrusive_ptr.test.cpp
)
