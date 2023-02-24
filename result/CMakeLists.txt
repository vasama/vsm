cmake_minimum_required(VERSION 3.23)

project(vsm_result)

if(PROJECT_IS_TOP_LEVEL)
	find_package(vsm::core REQUIRED)
	find_package(vsm::error_code REQUIRED)
endif()

add_library(vsm_result INTERFACE)

target_sources(vsm_result
	PUBLIC
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
		include/vsm/result.hpp
)

target_include_directories(vsm_result
	INTERFACE
		include
)

target_link_libraries(vsm_result
	INTERFACE
		vsm::core
		vsm::error_code
)

if(NOT PROJECT_IS_TOP_LEVEL)
	add_library(vsm::result ALIAS vsm_result)
endif()

#add_executable(vsm_result_test
#	source/result.test.cpp
#)
#target_link_libraries(vsm_result_test
#	PRIVATE
#		vsm_result
#)
