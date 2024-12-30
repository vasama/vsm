cmake_minimum_required(VERSION 3.24)
project(vsm_linear)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::linear

	HEADERS
		include/vsm/linear.hpp
)
