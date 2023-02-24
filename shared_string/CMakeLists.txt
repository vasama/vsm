cmake_minimum_required(VERSION 3.23.0)

project(vsm_shared_string)

if(PROJECT_IS_TOP_LEVEL)
	find_package(vsm-core REQUIRED)
	find_package(vsm-allocator REQUIRED)
	find_package(vsm-vector REQUIRED)
endif()

add_library(vsm_shared_string INTERFACE)

target_sources(vsm_shared_string
	INTERFACE
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
		include/vsm/shared_string.hpp
)

target_include_directories(vsm_shared_string
	INTERFACE
		include
)

target_link_libraries(vsm_shared_string
	INTERFACE
		vsm::core
		vsm::allocator
		vsm::vector
)

install(TARGETS vsm_shared_string FILE_SET HEADERS)

if(NOT PROJECT_IS_TOP_LEVEL)
	add_library(vsm::shared_string ALIAS vsm_shared_string)
endif()


add_executable(vsm_shared_string_test
	source/vsm/impl/shared_string.test.cpp
)
target_link_libraries(vsm_shared_string_test
	PRIVATE
		vsm_shared_string
		Catch2::Catch2WithMain
)
