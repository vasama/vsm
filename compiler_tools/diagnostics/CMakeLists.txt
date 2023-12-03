cmake_minimum_required(VERSION 3.23)

project(vsm_compilers_diagnostics)

vsm_add_library(vsm::compilers::diagnostics
	HEADERS
		include/vsm/compilers/diagnostics.hpp

	SOURCES
		source/vsm/compilers/impl/diagnostics.cpp

	HEADER_DEPENDENCIES
		vsm::compilers::source
		vsm::core
		vsm::hash_table
		vsm::tag_ptr
		vsm::vector
)
