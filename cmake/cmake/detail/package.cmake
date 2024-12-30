function(vsm_define_package name)
	# Load the package_info.json:
	set(package_info_file "${CMAKE_CURRENT_SOURCE_DIR}/package_info.json")
	if(NOT EXISTS "${package_info_file}")
		message(FATAL_ERROR "package_info.json not found")
	endif()
	file(READ "${package_info_file}" package_info)

	string(
		JSON package_info_name
		GET "${package_info}" package)

	# The specified package name must match the one in the package_info.json:
	if(NOT "${name}" STREQUAL "${package_info_name}")
		message(FATAL_ERROR "Package name does not match package_info.json.")
	endif()

	string(
		JSON requirements
		ERROR_VARIABLE json_error
		GET "${package_info}" requirements)

	# Find packages listed in package_info.json requirements:
	if(NOT "${requirements}" STREQUAL "requirements-NOTFOUND")
		get_property(
			root_name DIRECTORY "${CMAKE_SOURCE_DIR}"
			PROPERTY vsm_detail_root_name)

		string(JSON requirements_count LENGTH "${requirements}")
		math(EXPR requirements_count "${requirements_count} - 1")

		foreach(index RANGE "${requirements_count}")
			string(JSON requirement GET "${requirements}" "${index}")
			string(JSON require_name GET "${requirement}" "package")

			if("${require_name}" STREQUAL "vsm.cmake")
				continue()
			endif()

			if(NOT "${root_name}" STREQUAL "" AND "${require_name}" MATCHES "^${root_name}[\\.\\+\\-].+")
				continue()
			endif()
	
			string(
				JSON find_package_name
				ERROR_VARIABLE json_error
				GET "${requirement}" "find_package")
	
			if("${find_package_name}" STREQUAL "find_package-NOTFOUND")
				set(find_package_name "${require_name}")
			endif()
	
			find_package("${find_package_name}")
		endforeach()
	endif()

	set_property(
		DIRECTORY "${PROJECT_SOURCE_DIR}"
		PROPERTY vsm_detail_package_name "${name}")

	# Clear out the package setup script file:
	file(REMOVE "${PROJECT_BINARY_DIR}/${name}-setup.cmake")

	# Clear out the package setup script directory:
	file(REMOVE_RECURSE "${PROJECT_BINARY_DIR}/${name}-setup")
endfunction()

function(vsm_cmake_package_setup)
	cmake_parse_arguments(
		OPT
		""
		"NAME"
		"INCLUDE;CONTENT"
		${ARGN}
	)

	get_property(
		package_name
		DIRECTORY "${PROJECT_SOURCE_DIR}"
		PROPERTY vsm_detail_package_name)

	set(setup_file "${PROJECT_BINARY_DIR}/${package_name}-setup/${OPT_NAME}.cmake")
	if(NOT EXISTS "${setup_file}")
		file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/${package_name}-setup")
		file(WRITE "${setup_file}" "")
		install(FILES "${setup_file}" DESTINATION "cmake/${package_name}-setup")

		if(DEFINED OPT_INCLUDE)
			file(READ "${OPT_INCLUDE}" include_content)
			file(APPEND "${setup_file}" "${include_content}")
		endif()
	endif()
	file(APPEND "${setup_file}" ${OPT_CONTENT})

	set(setup_root "${PROJECT_BINARY_DIR}/${package_name}-setup.cmake")
	if(NOT EXISTS "${setup_root}")
		file(WRITE "${setup_root}" "")
		install(FILES "${setup_root}" DESTINATION "cmake")
	endif()
	file(APPEND "${setup_root}" "include(\"\${CMAKE_CURRENT_LIST_DIR}/${package_name}-setup/${OPT_NAME}.cmake\")\n")
endfunction()
