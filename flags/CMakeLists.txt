cmake_minimum_required(VERSION 3.23)

project(vsm_flags)

vsm_add_library(vsm_flags
	EXPORT_ALIAS vsm::flags

	HEADERS
		include/vsm/flags.hpp

	TEST_SOURCES
		source/vsm/test/flags.cpp
)
