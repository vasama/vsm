cmake_minimum_required(VERSION 3.24)

project(vsm_execution)

find_package(vsm-cmake REQUIRED)

vsm_find_sibling(vsm-core)
vsm_find_sibling(vsm-intrusive)
find_package(P2300 REQUIRED)

vsm_add_library(
	vsm::execution

	HEADERS
		include/vsm/execution/error.hpp
		include/vsm/execution/execution.hpp
		include/vsm/execution/run_queue.hpp

	HEADER_DEPENDENCIES
		vsm::core
		vsm::intrusive
		P2300::P2300

	TEST_SOURCES
		source/vsm/test/run_queue.cpp
)
