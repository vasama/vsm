cmake_minimum_required(VERSION 3.23)

project(vsm_cli)

if(PROJECT_IS_TOP_LEVEL)
	find_package(vsm-assert REQUIRED)
	find_package(vsm-core REQUIRED)
endif()

add_library(vsm_cli)

target_sources(vsm_cli
	PUBLIC
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
		include/vsm/cli/detail/base.hpp
		include/vsm/cli/app.hpp
		include/vsm/cli/error.hpp
		include/vsm/cli/group.hpp
		include/vsm/cli/option.hpp
		include/vsm/cli/parse.hpp
		include/vsm/cli/style.hpp
		include/vsm/cli.hpp

	PRIVATE
		source/vsm/cli/impl/app.cpp
		source/vsm/cli/impl/app.hpp
		source/vsm/cli/impl/group.cpp
		source/vsm/cli/impl/group.hpp
		source/vsm/cli/impl/option.cpp
		source/vsm/cli/impl/option.hpp
		source/vsm/cli/impl/resource.hpp
		source/vsm/cli/impl/style.cpp
)

target_include_directories(vsm_cli
	PUBLIC
		include
)

target_link_libraries(vsm_cli
	PUBLIC
		vsm::assert
		vsm::core
)

install(TARGETS vsm_cli FILE_SET HEADERS)

if(NOT PROJECT_IS_TOP_LEVEL)
	add_library(vsm::assert ALIAS vsm_cli)
endif()
