cmake_minimum_required(VERSION 3.24)

project(vsm_log)

vsm_find_sibling(vsm-core)

vsm_add_library(
	vsm::log

	HEADERS
		include/vsm/log.hpp
		include/vsm/log/level.hpp
		include/vsm/log/log.hpp

	HEADER_DEPENDENCIES
		vsm::core

	TEST_SOURCES
		source/vsm/test/log.cpp
)
