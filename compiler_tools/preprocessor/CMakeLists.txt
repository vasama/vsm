cmake_minimum_required(VERSION 3.23.0)

project(vsm_preprocessor)

vsm_add_library(vsm_preprocessor
	EXPORT_ALIAS vsm::preprocessor

	HEADERS
		include/vsm/preprocessor.hpp

	SOURCES
		source/vsm/impl/preprocessor.cpp

	HEADER_DEPENDENCIES
		vsm::flags

	SOURCE_DEPENDENCIES
		vsm::compilers::diagnostics
		vsm::inspect
)
