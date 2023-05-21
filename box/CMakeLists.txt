cmake_minimum_required(VERSION 3.23)

project(vsm_box)

vsm_add_library(vsm_box
	EXPORT_ALIAS vsm::box
	
	HEADERS
		include/vsm/box.hpp
)
