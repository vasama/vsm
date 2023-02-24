cmake_minimum_required(VERSION 3.24)

#set(VSM_CDP_INSTALL_DIR ${CMAKE_BINARY_DIR}/my_packages
#	CACHE PATH "The directory in which this provider installs packages."
#)

#list(APPEND CMAKE_MODULE_PATH ${VSM_CDP_INSTALL_DIR}/cmake)
#list(APPEND CMAKE_PREFIX_PATH ${VSM_CDP_INSTALL_DIR})

function(VSM_CDP_map var val def)
	#message(STATUS "VSM_CDP_map '${val}'")
	math(EXPR c "${ARGC} - 1")
	foreach(i RANGE 3 ${c} 2)
		math(EXPR j "${i} + 1")
		#message(STATUS "  VSM_CDP_map ARGV${i}='${ARGV${i}}' ARGV${j}='${ARGV${j}}'")
		if(val STREQUAL ${ARGV${i}})
			set(${var} ${ARGV${j}} PARENT_SCOPE)
			return()
		endif()
	endforeach()
	if("${def}" STREQUAL REQUIRED)
		message(FATAL_ERROR "Required mapping for '${val}' not found.")
	endif()
	set(${var} ${def} PARENT_SCOPE)
endfunction()

function(VSM_CDP_generate_profile config profile_path)
	set(profile "[settings]\n")

	# build_type
	string(APPEND profile "build_type=${config}\n")

	# arch
	string(TOLOWER ${CMAKE_CXX_COMPILER_ARCHITECTURE_ID} arch)
	VSM_CDP_map(arch ${arch} REQUIRED
		x64 x86_64
	)
	string(APPEND profile "arch=${arch}\n")

	# os
	string(APPEND profile "os=${CMAKE_SYSTEM_NAME}\n")

	# compiler
	string(TOLOWER ${CMAKE_CXX_COMPILER_ID} compiler)
	VSM_CDP_map(compiler ${compiler} ${compiler}
		"msvc" "Visual Studio"
	)
	string(APPEND profile "compiler=${compiler}\n")

	# compiler options
	if(${compiler} STREQUAL "Visual Studio")
		VSM_CDP_map(runtime_suffix ${config} ""
			Debug d
		)
		string(APPEND profile "compiler.runtime=MD${runtime_suffix}\n")

		VSM_CDP_map(compiler_version ${MSVC_TOOLSET_VERSION} REQUIRED
			80 8
			90 9
			100 10
			110 11
			120 12
			140 14
			141 15
			142 16
			143 17
		)
		string(APPEND profile "compiler.version=${compiler_version}\n")
	endif()
	if(${compiler} STREQUAL "gcc" OR ${compiler} STREQUAL "clang")
		#TODO: Detect stdlib.
		string(APPEND profile "compiler.stdlib=libstdc++\n")
		
		set(compiler_version ${CMAKE_CXX_COMPILER_VERSION})
		string(FIND ${compiler_version} . compiler_version_length)
		string(SUBSTRING ${compiler_version} 0 ${compiler_version_length} compiler_version)
		string(APPEND profile "compiler.version=${compiler_version}\n")
	endif()

	file(WRITE ${profile_path} ${profile})
endfunction()

macro(VSM_CDP method package_name)
	set(switches
		EXACT
		QUIET
		CONFIG
		NO_MODULE
		NO_POLICY_SCOPE
		BYPASS_PROVIDER
		NO_DEFAULT_PATH
		NO_PACKAGE_ROOT_PATH
		NO_CMAKE_PATH
		NO_CMAKE_ENVIRONMENT_PATH
		NO_SYSTEM_ENVIRONMENT_PATH
		NO_CMAKE_PACKAGE_REGISTRY
		NO_CMAKE_BUILDS_PATH
		NO_CMAKE_SYSTEM_PATH
		NO_CMAKE_INSTALL_PREFIX
		NO_CMAKE_SYSTEM_PACKAGE_REGISTRY
		CMAKE_FIND_ROOT_PATH_BOTH
		ONLY_CMAKE_FIND_ROOT_PATH
		NO_CMAKE_FIND_ROOT_PATH
	)
	set(options1
		REGISTRY_VIEW
	)
	set(optionsN
		COMPONENTS
		OPTIONAL_COMPONENTS
		NAMES
		CONFIGS
		HINTS
		PATHS
		PATH_SUFFIXES
	)
	cmake_parse_arguments(VSM_CDP_OPT "${switches}" "${options1}" "${optionsN}" ${ARGN})

	if(NOT DEFINED VSM_CDP_OPT_UNPARSED_ARGUMENTS)
		message(ERROR "Package version is required.")
		return()
	endif()

	list(GET VSM_CDP_OPT_UNPARSED_ARGUMENTS 0 package_version)

	if(DEFINED CMAKE_CONFIGURATION_TYPES)
		set(configs ${CMAKE_CONFIGURATION_TYPES})
	else()
		set(configs ${CMAKE_BUILD_TYPE})
	endif()

	foreach(config ${configs})
		VSM_CDP_map(config ${config} Release Debug Debug)

		if(DEFINED VSM_CDP_CFG_${config})
			continue()
		endif()
		set(VSM_CDP_CFG_${config} 1)

		set(profile_path "${CMAKE_BINARY_DIR}/conanprofile-${config}.txt")
		if(NOT EXISTS "${profile_path}")
			VSM_CDP_generate_profile(${config} ${profile_path})
		endif()

		execute_process(
			OUTPUT_QUIET
			COMMAND
				conan install "${package_name}/${package_version}@"
				--profile "${profile_path}"
				--build missing
			COMMAND_ERROR_IS_FATAL ANY
		)
	endforeach()

	set(${package_name}_FOUND TRUE)
endmacro()

cmake_language(
	SET_DEPENDENCY_PROVIDER VSM_CDP
	SUPPORTED_METHODS FIND_PACKAGE
)
