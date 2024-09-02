cmake_minimum_required(VERSION 3.24)

project(vsm_shared_span)

find_package(vsm-cmake REQUIRED)

vsm_find_sibling(vsm-core)

vsm_add_library(
	vsm::shared_span

	HEADERS
		include/vsm/shared_span.hpp

	HEADER_DEPENDENCIES
		vsm::core
)
