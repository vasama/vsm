cmake_minimum_required(VERSION 3.23.0)

project(vsm_propagate_const)

if(PROJECT_IS_TOP_LEVEL)
	find_package(vsm-core REQUIRED)
endif()

add_library(vsm_propagate_const INTERFACE)

target_sources(vsm_propagate_const
	INTERFACE
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
		include/vsm/propagate_const.hpp
)

target_include_directories(vsm_propagate_const
	INTERFACE
		include
)

target_link_libraries(vsm_propagate_const
	INTERFACE
		vsm::core
)

install(TARGETS vsm_propagate_const FILE_SET HEADERS)

if(NOT PROJECT_IS_TOP_LEVEL)
	add_library(vsm::propagate_const ALIAS vsm_propagate_const)
endif()


add_executable(vsm_propagate_const_test
	source/vsm/impl/propagate_const.test.cpp
)
target_include_directories(vsm_propagate_const_test
	PRIVATE
		source
)
target_link_libraries(vsm_propagate_const_test
	PRIVATE
		vsm_propagate_const
		Catch2::Catch2WithMain
)
