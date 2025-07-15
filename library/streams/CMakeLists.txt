cmake_minimum_required(VERSION 3.24)
project(vsm_streams)

find_package(vsm.cmake REQUIRED)
vsm_define_package(vsm.streams)

vsm_add_library(
	vsm::streams

	HEADERS
		include/vsm/streams/any.hpp
		include/vsm/streams/c_streams.hpp
		include/vsm/streams/streams.hpp

	SOURCES
		source/vsm/streams/impl/c_streams.cpp

	HEADER_DEPENDENCIES
		vsm::any
		vsm::core
		vsm::result
)
