cmake_minimum_required(VERSION 3.23.0)

project(vsm_algorithm)

add_library(vsm_algorithm INTERFACE)

target_sources(vsm_algorithm
	INTERFACE
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
		include/vsm/algorithm/remove_unstable.hpp
)

target_include_directories(vsm_algorithm
	INTERFACE
		include
)

install(TARGETS vsm_algorithm FILE_SET HEADERS)

if(NOT PROJECT_IS_TOP_LEVEL)
	add_library(vsm::algorithm ALIAS vsm_algorithm)
endif()


add_executable(vsm_algorithm_test
	source/vsm/algorithm/impl/remove_unstable.test.cpp
)
target_link_libraries(vsm_algorithm_test
	PRIVATE
		vsm_algorithm
		Catch2::Catch2WithMain
)
