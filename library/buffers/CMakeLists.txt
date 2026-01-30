cmake_minimum_required(VERSION 3.24)
project(vsm_buffers)

find_package(vsm.cmake REQUIRED)
vsm_define_package(vsm.buffers)

vsm_add_library(
	vsm::buffers

	HEADERS
		include/vsm/buffers.hpp

	HEADER_DEPENDENCIES
		vsm::core
)
