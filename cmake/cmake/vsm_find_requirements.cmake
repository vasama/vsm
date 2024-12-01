include_guard(GLOBAL)

function(vsm_detail_find_requirement package version)
	cmake_parse_arguments(
		VSM_OPT
		"TEST;BUILD;PRIVATE;HEADER_ONLY"
		"USER;CHANNEL;FIND"
		""
		${ARGN}
	)

	if(DEFINED VSM_OPT_UNPARSED_ARGUMENTS)
		message(SEND_ERROR "Unrecognized requirement options: ${VSM_OPT_UNPARSED_ARGUMENTS}")
	endif()

	if(NOT DEFINED VSM_OPT_FIND)
		# Skip packages starting with ~ ${root_name}-
		if(NOT "${root_name}" STREQUAL "" AND "${package}" MATCHES "^${root_name}[\\.\\+\\-].+")
			return()
		endif()

		set(VSM_OPT_FIND "${package}")
	endif()

	set(find_package_args "")
	if("${version}" MATCHES "^[0-9]+(\\.[0-9]+(\\.[0-9]+(\\.[0-9]+)?)?)?$")
		list(APPEND find_package_args "${version}")
	endif()

	set(find_package_quiet ON)
	if(${CMAKE_MAJOR_VERSION} EQUAL 3 AND ${CMAKE_MINOR_VERSION} GREATER_EQUAL 25)
		cmake_language(GET_MESSAGE_LOG_LEVEL log_level)
		if("${log_level}" STREQUAL VERBOSE OR "${log_level}" STREQUAL TRACE)
			set(find_package_quiet OFF)
		endif()
	endif()

	if(find_package_quiet)
		list(APPEND find_package_args QUIET)
	endif()

	find_package("${VSM_OPT_FIND}" ${find_package_args} REQUIRED)
endfunction()

function(vsm_find_requirements)
	get_property(root_name DIRECTORY PROPERTY vsm_detail_root_name)

	set(requirements_file "${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt")
	if(NOT EXISTS "${requirements_file}")
		message(FATAL_ERROR "Requirements file not found: '${requirements_file}'")
	endif()
	file(STRINGS "${requirements_file}" requirements)

	foreach(requirement ${requirements})
		string(STRIP "${requirement}" requirement)
		string(REGEX REPLACE "#.+$" "" requirement "${requirement}")

		if("${requirement}" STREQUAL "")
			continue()
		endif()

		cmake_language(EVAL CODE "set(requirement ${requirement})")
		vsm_detail_find_requirement(${requirement})
	endforeach()
endfunction()
