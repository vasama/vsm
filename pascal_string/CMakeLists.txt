cmake_minimum_required(VERSION 3.23.0)

project(vsm_pascal_string)

if(PROJECT_IS_TOP_LEVEL)
	find_package(vsm-core REQUIRED)
endif()

add_library(vsm_pascal_string INTERFACE)

target_sources(vsm_pascal_string
	INTERFACE
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
		include/vsm/pascal_string.hpp
)

target_include_directories(vsm_pascal_string
	INTERFACE
		include
)

target_link_libraries(vsm_pascal_string
	INTERFACE
		vsm::core
)

install(TARGETS vsm_pascal_string FILE_SET HEADERS)

if(NOT PROJECT_IS_TOP_LEVEL)
	add_library(vsm::pascal_string ALIAS vsm_pascal_string)
endif()


add_executable(vsm_pascal_string_test
	source/vsm/impl/pascal_string.test.cpp
)
target_include_directories(vsm_pascal_string_test
	PRIVATE
		source
)
target_link_libraries(vsm_pascal_string_test
	PRIVATE
		vsm_pascal_string
		Catch2::Catch2WithMain
)
