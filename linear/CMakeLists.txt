cmake_minimum_required(VERSION 3.23)

project(vsm_linear)

add_library(vsm_linear INTERFACE)

target_sources(vsm_linear
	INTERFACE
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
		include/vsm/linear.hpp
)

target_include_directories(vsm_linear
	INTERFACE
		include
)

install(TARGETS vsm_linear FILE_SET HEADERS)

if(NOT PROJECT_IS_TOP_LEVEL)
	add_library(vsm::linear ALIAS vsm_linear)
endif()
