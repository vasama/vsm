cmake_minimum_required(VERSION 3.23.0)

project(vsm_compiler_tools)

vsm_find_sibling(vsm-core)
vsm_find_sibling(vsm-propagate_const)

vsm_add_library(vsm_compiler_tools
	EXPORT_ALIAS vsm::compiler_tools

	HEADERS
		include/vsm/compilers/parser.hpp
		include/vsm/compilers/syntax.hpp

	HEADER_DEPENDENCIES
		vsm::core
		vsm::propagate_const
)
