cmake_minimum_required(VERSION 3.23)

project(vsm_hash)

vsm_add_library(vsm_math
	EXPORT_ALIAS vsm::math

	HEADERS
		include/vsm/math.hpp
)
