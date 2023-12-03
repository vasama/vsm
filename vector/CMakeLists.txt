cmake_minimum_required(VERSION 3.23)

project(vsm_vector)

if(PROJECT_IS_TOP_LEVEL)
	find_package(vsm-core REQUIRED)
	find_package(vsm-algorithm REQUIRED)
	find_package(vsm-allocator REQUIRED)
endif()

add_library(vsm_vector INTERFACE)

target_sources(vsm_vector
	INTERFACE
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
		include/vsm/vector.hpp
)

target_include_directories(vsm_vector
	INTERFACE
		include
)

target_link_libraries(vsm_vector
	INTERFACE
		vsm::core
		vsm::algorithm
		vsm::allocator
)

install(TARGETS vsm_vector FILE_SET HEADERS)

if(NOT PROJECT_IS_TOP_LEVEL)
	add_library(vsm::vector ALIAS vsm_vector)
endif()


add_executable(vsm_vector_test
	source/vsm/impl/vector.test.cpp
)
target_link_libraries(vsm_vector_test
	PRIVATE
		vsm_vector
		Catch2::Catch2WithMain
)
