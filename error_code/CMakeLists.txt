cmake_minimum_required(VERSION 3.23)

project(vsm_error_code)

vsm_add_library(vsm_error_code
	EXPORT_ALIAS vsm::error_code

	HEADERS
		include/vsm/error_code.hpp
		include/vsm/std_error_code.hpp

	SOURCES
		source/vsm/impl/error_code.cpp
		source/vsm/impl/std_error_code.cpp

	TEST_SOURCES
		source/vsm/test/error_code.cpp
)
