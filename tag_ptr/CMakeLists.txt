cmake_minimum_required(VERSION 3.23.0)

project(vsm_tag_ptr)

if(PROJECT_IS_TOP_LEVEL)
	find_package(vsm-core REQUIRED)
endif()

add_library(vsm_tag_ptr INTERFACE)

target_sources(vsm_tag_ptr
	PUBLIC
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
		include/vsm/tag_ptr.hpp
)

target_include_directories(vsm_tag_ptr
	INTERFACE
		include
)

target_link_libraries(vsm_tag_ptr
	INTERFACE
		vsm::core
)

install(TARGETS vsm_tag_ptr FILE_SET HEADERS)

if(NOT PROJECT_IS_TOP_LEVEL)
	add_library(vsm::tag_ptr ALIAS vsm_tag_ptr)
endif()


add_executable(vsm_tag_ptr_test
	source/vsm/impl/tag_ptr.test.cpp
)
target_link_libraries(vsm_tag_ptr_test
	PRIVATE
		vsm_tag_ptr
		Catch2::Catch2WithMain
)
