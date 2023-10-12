cmake_minimum_required(VERSION 3.23.0)

project(vsm_math)

vsm_add_library2(vsm::math
	HEADERS
		include/vsm/math.hpp

	TEST_SOURCES
		source/vsm/test/math.cpp
)
