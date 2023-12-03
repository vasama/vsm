cmake_minimum_required(VERSION 3.23)

project(vsm_allocator)

add_library(vsm_allocator INTERFACE)

target_sources(vsm_allocator
	INTERFACE
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
		include/vsm/allocator.hpp
		include/vsm/default_allocator.hpp
		include/vsm/new_allocator.hpp
)

target_include_directories(vsm_allocator
	INTERFACE
		include
)

install(TARGETS vsm_allocator FILE_SET HEADERS)

if(NOT PROJECT_IS_TOP_LEVEL)
	add_library(vsm::allocator ALIAS vsm_allocator)
endif()


add_executable(vsm_allocator_test
	source/vsm/impl/allocator.test.cpp
)
target_link_libraries(vsm_allocator_test
	PRIVATE
		vsm_allocator
		Catch2::Catch2WithMain
)
