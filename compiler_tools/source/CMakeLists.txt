cmake_minimum_required(VERSION 3.23.0)

project(vsm_compilers_source)

vsm_add_library(vsm_compilers_source
	EXPORT_ALIAS vsm::compilers::source

	HEADERS
		include/vsm/compilers/source.hpp
)
