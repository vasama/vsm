cmake_minimum_required(VERSION 3.24)

project(vsm_zstring_view)

vsm_find_sibling(vsm-core)

vsm_add_library(vsm::zstring_view
	HEADERS
		include/vsm/zstring_view.hpp

	HEADER_DEPENDENCIES
		vsm::core
)
