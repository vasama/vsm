cmake_minimum_required(VERSION 3.24)
project(vsm_pointer_tagging)

find_package(vsm.cmake REQUIRED)
vsm_define_package(vsm.pointer_tagging)

vsm_add_library(
	vsm::pointer_tagging

	HEADERS
		include/vsm/detail/atomic_pointer_tag_pair.hpp
		include/vsm/pointer_tag_pair.hpp
		include/vsm/tagged_ptr.hpp
		include/vsm/variant_ptr.hpp

	VISUALIZERS
		visualizers/vsm/pointer_tagging.natvis

	HEADER_LINK_LIBRARIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/pointer_tag_pair.cpp
		source/vsm/test/tagged_ptr.cpp

	TEST_LINK_LIBRARIES
		vsm::testing::core
)
