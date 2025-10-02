function(vsm_add_cmake)
	cmake_parse_arguments(
		VSM_OPT
		""
		"FORCE_INCLUDE;PLATFORM"
		""
		${ARGN}
	)

	set(script_files "${VSM_OPT_UNPARSED_ARGUMENTS}")
	if(DEFINED VSM_OPT_FORCE_INCLUDE)
		list(APPEND script_files "${VSM_OPT_FORCE_INCLUDE}")
	endif()

	vsm_detail_add_directory_files(cmake_scripts "${script_files}")

	get_property(
		"install"
		DIRECTORY "${PROJECT_SOURCE_DIR}"
		PROPERTY vsm_detail_install_cmake_scripts
	)

	if("${install}" STREQUAL "install-NOTFOUND")
		set_property(
			DIRECTORY "${PROJECT_SOURCE_DIR}"
			PROPERTY vsm_detail_install_cmake_scripts ""
		)

		install(FILES "${script_files}" DESTINATION "cmake")
	endif()

	if("${VSM_OPT_FORCE_INCLUDE}")
		include("${CMAKE_CURRENT_SOURCE_DIR}/${VSM_OPT_FORCE_INCLUDE}")
	endif()
endfunction()
