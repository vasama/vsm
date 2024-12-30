function(vsm_detail_read_package_info package_path out_package_name out_requirements)
	cmake_path(ABSOLUTE_PATH package_path NORMALIZE)

	set(package_name "")
	set(requirements "")

	set(inherit OFF)
	while(ON)
		set(package_info_path "${package_path}/package_info.json")
		if(EXISTS "${package_info_path}")
			file(READ "${package_info_path}" package_info)

			string(
				JSON layer_package_name
				ERROR_VARIABLE json_error
				GET "${package_info}" "package")

			if("${package}" STREQUAL "" AND NOT "${layer_package_name}" STREQUAL "package-NOTFOUND")
				set(package_name "${layer_package_name}")
			endif()

			string(
				JSON layer_requirements
				ERROR_VARIABLE json_error
				GET "${package_info}" "requirements")

			if(NOT "${layer_requirements}" STREQUAL "requirements-NOTFOUND")
				string(JSON requirement_count LENGTH "${layer_requirements}")
				if("${requirement_count}" GREATER 0)
					math(EXPR last_requirement_index "${requirement_count} - 1")
					foreach(requirement_index RANGE "${last_requirement_index}")
						string(JSON requirement GET "${layer_requirements}" "${requirement_index}")
						string(JSON requirement_name GET "${requirement}" "package")
						if(NOT "${requirement_name}" STREQUAL "vsm.cmake")
							list(APPEND requirements "${requirement}")
						endif()
					endforeach()
				endif()
			endif()

			string(
				JSON inherit
				ERROR_VARIABLE json_error
				GET "${package_info}" "inherit")

			if(NOT "${inherit}" STREQUAL "ON")
				break()
			endif()
		else()
			if(NOT "${inherit}")
				message(FATAL_ERROR "File not found: ${package_info_path}")
			endif()
		endif()

		cmake_path(GET package_path PARENT_PATH parent_path)
		if("${parent_path}" STREQUAL "" OR "${parent_path}" STREQUAL "${package_path}")
			message(FATAL_ERROR "Invalid inheritance: ${package_info_path}")
		else()
			set(package_path "${parent_path}")
		endif()
	endwhile()

	if(NOT "${package_name}" STREQUAL "")
		set("${out_package_name}" "${package_name}" PARENT_SCOPE)
	endif()

	set("${out_requirements}" "${requirements}" PARENT_SCOPE)
endfunction()

function(vsm_define_package name)
	cmake_parse_arguments(
		OPT
		"SETUP_SCRIPT"
		""
		""
		${ARGN}
	)

	if(DEFINED OPT_UNPARSED_ARGUMENTS)
		message(SEND_ERROR "vsm_define_package: unrecognized arguments: ${OPT_UNPARSED_ARGUMENTS}")
	endif()

	get_property(
		root_name
		DIRECTORY "${CMAKE_SOURCE_DIR}"
		PROPERTY vsm_detail_root_name
	)

	# Read the package_info.json:
	vsm_detail_read_package_info(
		"${CMAKE_CURRENT_SOURCE_DIR}"
		package_info_name
		package_info_requirements
	)

	# The specified package name must match the one in the package_info.json:
	if(NOT "${name}" STREQUAL "${package_info_name}")
		message(SEND_ERROR "Package name does not match package_info.json: '${name}'/'${package_info_name}'")
	endif()

	# Find packages listed in package_info.json requirements:
	foreach(requirement IN LISTS package_info_requirements)
		string(JSON requirement_name GET "${requirement}" "package")
		if(NOT "${root_name}" STREQUAL "" AND "${requirement_name}" MATCHES "^${root_name}\\..+")
			continue()
		endif()

		string(
			JSON requirement_find_package
			ERROR_VARIABLE json_error
			GET "${requirement}" "find_package")

		set(find_package_name "${requirement_name}")
		if(NOT "${requirement_find_package}" STREQUAL "find_package-NOTFOUND")
			string(
				JSON requirement_find_package_type
				ERROR_VARIABLE json_error
				GET "${requirement}" "find_package")

			if("${requirement_find_package_type}" STREQUAL "NULL")
				continue()
			endif()

			set(find_package_name "${requirement_find_package}")
		endif()

		find_package("${find_package_name}")
	endforeach()

	set_property(
		DIRECTORY "${PROJECT_SOURCE_DIR}"
		PROPERTY vsm_detail_package_name "${name}")

	# Clear out the package setup script file:
	file(REMOVE "${PROJECT_BINARY_DIR}/${name}-setup.cmake")

	# Clear out the package setup script directory:
	file(REMOVE_RECURSE "${PROJECT_BINARY_DIR}/${name}-setup")

	if(OPT_SETUP_SCRIPT)
		vsm_add_cmake_package_setup(
			NAME vsm_setup_script
			CONTENT "include(${name})\n"
		)
	endif()
endfunction()

function(vsm_add_cmake_package_setup)
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
		PROPERTY vsm_detail_package_name
	)

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
