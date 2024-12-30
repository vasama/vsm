cmake_minimum_required(VERSION 3.24)
project(vsm_function)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::function

	HEADERS
		include/vsm/detail/function.hpp
		include/vsm/detail/function_view.hpp

		include/vsm/function.hpp
		include/vsm/function_view.hpp
		include/vsm/inplace_function.hpp

	HEADER_DEPENDENCIES
		vsm::allocator
		vsm::core

	TEST_SOURCES
		source/vsm/test/function_view.cpp
		source/vsm/test/function.cpp
		source/vsm/test/inplace_function.cpp

	TEST_DEPENDENCIES
		vsm::testing::allocator
)
