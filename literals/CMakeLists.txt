cmake_minimum_required(VERSION 3.24)

project(vsm_literals)

vsm_add_library(vsm::literals
	HEADERS
		include/vsm/literals.hpp

	TEST_SOURCES
		source/vsm/test/literals.cpp
)
