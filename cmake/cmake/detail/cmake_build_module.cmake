include_guard(GLOBAL)

function(vsm_detail_add_cmake_build_module name)
	cmake_parse_arguments(
		VSM_OPT
		""
		"COPY_FILE"
		""
		${ARGN}
	)

	set(module "${PROJECT_BINARY_DIR}/vsm_cbm/${name}.cmake")

	get_property(
		has_module
		DIRECTORY "${PROJECT_BINARY_DIR}"
		PROPERTY "vsm_detail_cbm_${name}"
	)

	if(NOT ${has_module})
		set_property(
			DIRECTORY "${PROJECT_BINARY_DIR}"
			PROPERTY "vsm_detail_cbm_${name}" ON
		)

		if(DEFINED VSM_OPT_COPY_FILE)
			file(COPY_FILE "${VSM_OPT_COPY_FILE}" "${module}" RESULT copy_file_result)

			if(NOT "${copy_file_result}" STREQUAL 0)
				message(SEND_ERROR "${copy_file_result}")
			endif()
		else()
			file(WRITE "${module}" "")
		endif()

		install(
			FILES "${module}"
			DESTINATION cmake_build_modules
		)
	endif()

	file(APPEND "${module}" ${VSM_OPT_UNPARSED_ARGUMENTS})
endfunction()
