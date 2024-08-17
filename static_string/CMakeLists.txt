cmake_minimum_required(VERSION 3.24)

project(vsm_static_string)

vsm_add_library(
	vsm::static_string

	HEADERS
		include/vsm/static_string.hpp
)
