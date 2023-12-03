cmake_minimum_required(VERSION 3.23)

project(vsm_any)

if(PROJECT_IS_TOP_LEVEL)
	find_package(vsm-core REQUIRED)
endif()

add_library(vsm_any INTERFACE)

target_sources(vsm_any
	INTERFACE
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
		include/vsm/any_interface.hpp
		include/vsm/any_ref.hpp
)

target_include_directories(vsm_any
	INTERFACE
		include
)

target_link_libraries(vsm_any
	INTERFACE
		vsm::core
)

install(TARGETS vsm_any FILE_SET HEADERS)

if(NOT PROJECT_IS_TOP_LEVEL)
	add_library(vsm::any ALIAS vsm_any)
endif()


add_executable(vsm_any
	source/vsm/impl/any_ref.test.cpp
)
target_link_libraries(vsm_any
	PRIVATE
		vsm_any
		Catch2::Catch2WithMain
)
