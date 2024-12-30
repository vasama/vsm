include_guard(GLOBAL)

function(vsm_find_requirements)
	get_property(root_name DIRECTORY PROPERTY vsm_detail_root_name)
	set(package_info_path "${CMAKE_CURRENT_SOURCE_DIR}/package_info.json")

	if(NOT EXISTS "${package_info_path}")
		return()
	endif()

	file(READ "${package_info_path}" package_info)

	string(
		JSON requirements
		ERROR_VARIABLE json_error
		GET "${package_info}" "requirements")

	if("${requirements}" STREQUAL "requirements-NOTFOUND")
		return()
	endif()

	string(JSON requirements_count LENGTH "${requirements}")
	math(EXPR requirements_count "${requirements_count} - 1")

	foreach(index RANGE "${requirements_count}")
		string(JSON requirement GET "${requirements}" "${index}")
		string(JSON package_name GET "${requirement}" "package")

		if(NOT "${root_name}" STREQUAL "" AND "${package_name}" MATCHES "^${root_name}[\\.\\+\\-].+")
			continue()
		endif()

		string(
			JSON find_package_name
			ERROR_VARIABLE json_error
			GET "${requirement}" "find_package")

		if(NOT "${find_package_name}" STREQUAL "find_package-NOTFOUND")
			set(package_name "${find_package_name}")
		endif()

		find_package("${package_name}")
	endforeach()
endfunction()
