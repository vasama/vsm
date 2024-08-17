cmake_minimum_required(VERSION 3.24)

project(vsm_function)

vsm_find_sibling(vsm-allocator)
vsm_find_sibling(vsm-core)

vsm_add_library(
	vsm::function

	HEADERS
		include/vsm/detail/function.hpp
		include/vsm/detail/function_view.hpp

		include/vsm/function.hpp
		include/vsm/function_view.hpp
		include/vsm/static_function.hpp

	HEADER_DEPENDENCIES
		vsm::allocator
		vsm::core

	TEST_SOURCES
		source/vsm/test/function_view.cpp
		source/vsm/test/function.cpp
		source/vsm/test/static_function.cpp
)
