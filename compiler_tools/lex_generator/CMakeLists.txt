cmake_minimum_required(VERSION 3.23.0)

project(vsm_lex_generator)

vsm_find_sibling(vsm-core)

vsm_add_library(vsm_lex_generator
	EXPORT_ALIAS vsm::lex_generator

	HEADERS
		include/vsm/lex_generator.hpp

	SOURCES
		source/vsm/impl/codegen.cpp
		source/vsm/impl/codegen.hpp
		source/vsm/impl/lexer_bootstrap.cpp
		source/vsm/impl/lexer_bootstrap.hpp
		source/vsm/impl/lexer.hpp
		source/vsm/impl/semantics.cpp
		source/vsm/impl/semantics.hpp
		source/vsm/impl/syntax.cpp
		source/vsm/impl/syntax.hpp

	HEADER_DEPENDENCIES
		vsm::core

	SOURCE_DEPENDENCIES
		vsm::vector
		vsm::compiler_tools
)
