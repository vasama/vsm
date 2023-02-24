cmake_minimum_required(VERSION 3.24)
project(vsm_partial)

find_package(vsm-cmake REQUIRED)

vsm_add_library(
	vsm::partial

	HEADERS
		include/vsm/partial.hpp
)
