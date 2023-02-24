cmake_minimum_required(VERSION 3.21)

project(vsm_function)

if(PROJECT_IS_TOP_LEVEL)
	find_package(vsm-core REQUIRED)
endif()

add_library(vsm_function INTERFACE)

target_include_directories(vsm_function
	INTERFACE
		include
)

target_link_libraries(vsm_function
	INTERFACE
		vsm::core
)

install(TARGETS vsm_function FILE_SET HEADERS)

if(NOTPROJECT_IS_TOP_LEVEL)
	add_library(vsm::function ALIAS vsm_function)
endif()


add_executable(vsm_function_test
	source/function_view.test.cpp
	source/function.test.cpp
	source/static_function.test.cpp
)
target_link_libraries(vsm_function_test
	PRIVATE
		vsm_function
		Catch2::Catch2WithMain
)
