cmake_minimum_required(VERSION 3.23)

project(vsm-cmake_test)

include(vsm)

vsm_add_library(vsm::cmake_test
	HEADERS
		include/cmake_test.hpp
)
