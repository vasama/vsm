cmake_minimum_required(VERSION 3.24)

project(vsm_container_core)

vsm_find_sibling(vsm-core)

vsm_add_library(
	vsm::container_core

	HEADERS
		include/vsm/insert_result.hpp
		include/vsm/key_selector.hpp
		include/vsm/key_value_pair.hpp

	HEADER_DEPENDENCIES
		vsm::core
)
