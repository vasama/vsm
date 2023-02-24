cmake_minimum_required(VERSION 3.23.0)

project(vsm_unique_resource)

if(PROJECT_IS_TOP_LEVEL)
	find_package(vsm-core REQUIRED)
endif()

add_library(vsm_unique_resource INTERFACE)

target_sources(vsm_unique_resource
	PUBLIC
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
		include/vsm/unique_resource.hpp
)

target_include_directories(vsm_unique_resource
	INTERFACE
		include
)

target_link_libraries(vsm_unique_resource
	INTERFACE
		vsm::core
)

install(TARGETS vsm_unique_resource FILE_SET HEADERS)

if(NOT PROJECT_IS_TOP_LEVEL)
	add_library(vsm::unique_resource ALIAS vsm_unique_resource)
endif()


add_executable(vsm_unique_resource_test
	source/unique_resource.test.cpp
)

target_link_libraries(vsm_unique_resource_test
	PRIVATE
		vsm_unique_resource
		Catch2::Catch2WithMain
)
