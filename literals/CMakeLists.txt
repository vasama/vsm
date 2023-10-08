cmake_minimum_required(VERSION 3.23.0)

project(vsm_literals)

vsm_add_library2(vsm::literals
	HEADERS
		include/vsm/literals.hpp

	TEST_SOURCES
		source/vsm/test/literals.cpp
)
