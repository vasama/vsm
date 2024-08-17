cmake_minimum_required(VERSION 3.24)

project(vsm_cli)

vsm_add_library(
	vsm::cli

	HEADERS
		include/vsm/cli.hpp
		include/vsm/cli/app.hpp
		include/vsm/cli/detail/base.hpp
		include/vsm/cli/error.hpp
		include/vsm/cli/group.hpp
		include/vsm/cli/option.hpp
		include/vsm/cli/parse.hpp
		include/vsm/cli/style.hpp

	SOURCES
		source/vsm/cli/impl/app.cpp
		source/vsm/cli/impl/app.hpp
		source/vsm/cli/impl/group.cpp
		source/vsm/cli/impl/group.hpp
		source/vsm/cli/impl/option.cpp
		source/vsm/cli/impl/option.hpp
		source/vsm/cli/impl/resource.hpp
		source/vsm/cli/impl/style.cpp

	HEADER_DEPENDENCIES
		vsm::assert
		vsm::core
)
