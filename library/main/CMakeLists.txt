cmake_minimum_required(VERSION 3.24)
project(vsm_main)

find_package(vsm.cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::main

	HEADERS
		include/vsm/main.h
		include/vsm/main.hpp

	SOURCES
		source/vsm/impl/main_entrypoint.cpp
		source/vsm/impl/main.cpp
		source/vsm/impl/nothrow_main.cpp

	HEADER_DEPENDENCIES
		vsm::result
)

