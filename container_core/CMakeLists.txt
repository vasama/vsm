cmake_minimum_required(VERSION 3.23.0)

project(vsm_container_core)

add_library(vsm_container_core INTERFACE)

target_sources(vsm_container_core
	INTERFACE
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
		include/vsm/insert_result.hpp
		include/vsm/key_selector.hpp
		include/vsm/key_value_pair.hpp
)

target_include_directories(vsm_container_core
	INTERFACE
		include
)

install(TARGETS vsm_container_core FILE_SET HEADERS)

if(NOT PROJECT_IS_TOP_LEVEL)
	add_library(vsm::container_core ALIAS vsm_container_core)
endif()
