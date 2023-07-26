cmake_minimum_required(VERSION 3.23)

project(vsm_async)

vsm_find_sibling(vsm-core)
vsm_find_sibling(vsm-intrusive)

vsm_add_library(vsm_async
	EXPORT_ALIAS vsm::async

	HEADERS
		include/vsm/async/timer.hpp

	HEADER_DEPENDENCIES
		vsm::core
		vsm::intrusive

	TEST_SOURCES
		source/vsm/async/test/timer.cpp
)
