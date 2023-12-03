cmake_minimum_required(VERSION 3.23)

project(vsm_lex)

vsm_find_sibling(vsm-core)

vsm_add_library(vsm::lex
	HEADERS
		include/vsm/lex.hpp

	SOURCES
		source/vsm/impl/lex.cpp

	HEADER_DEPENDENCIES
		vsm::core
)
