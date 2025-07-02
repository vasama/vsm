cmake_minimum_required(VERSION 3.24)
project(vsm_inplace_vector)

find_package(vsm.cmake REQUIRED)
vsm_define_package(vsm.inplace_vector)

vsm_add_library(vsm::inplace_vector
	HEADERS
		include/vsm/inplace_vector.hpp

	HEADER_DEPENDENCIES
		vsm::core
)
