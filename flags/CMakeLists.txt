cmake_minimum_required(VERSION 3.21)

project(vsm_flags)

if(PROJECT_IS_TOP_LEVEL)
	find_package(vsm-core REQUIRED)
endif()

add_library(vsm_flags INTERFACE)

target_sources(vsm_flags
	PUBLIC
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
		include/vsm/flags.hpp
)

target_include_directories(vsm_flags
	INTERFACE
		include
)

install(TARGETS vsm_flags FILE_SET HEADERS)


if(NOT PROJECT_IS_TOP_LEVEL)
	add_library(vsm::flags ALIAS vsm_flags)
endif()


#add_executable(vsm_flags_test
#	source/flags.test.cpp
#)
#target_link_libraries(vsm_flags_test
#	PRIVATE
#		vsm_flags
#		Catch2WithMain
#)
