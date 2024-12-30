cmake_minimum_required(VERSION 3.24)
project(vsm_container_core)

find_package(vsm-cmake REQUIRED)
vsm_find_requirements()

vsm_add_library(
	vsm::container_core

	HEADERS
		include/vsm/detail/array_view.hpp

		include/vsm/insert_result.hpp
		include/vsm/key_selector.hpp
		include/vsm/key_value_pair.hpp

	HEADER_DEPENDENCIES
		vsm::core
)
