include_guard(GLOBAL)

function(vsm_detail_add_force_include name)
	cmake_parse_arguments(
		VSM_OPT
		""
		"COPY_FILE"
		""
		${ARGN}
	)

	set(root_force_include "${PROJECT_BINARY_DIR}/vsm_force_include.cmake")

	get_property(
		has_root_force_include
		DIRECTORY "${CMAKE_SOURCE_DIR}"
		PROPERTY "vsm_detail_force_include"
	)

	if(NOT ${has_root_force_include})
		set(root_force_include)

		set_property(
			DIRECTORY "${CMAKE_SOURCE_DIR}"
			PROPERTY "vsm_detail_force_include" ON
		)

		install(
			FILES "${root_force_include}"
			DESTINATION ""
		)
	endif()

	set(force_include "${PROJECT_BINARY_DIR}/vsm_force_include/${name}.cmake")

	get_property(
		has_force_include
		DIRECTORY "${CMAKE_SOURCE_DIR}"
		PROPERTY "vsm_detail_force_include_${name}"
	)

	if(NOT ${has_force_include})
		set_property(
			DIRECTORY "${CMAKE_SOURCE_DIR}"
			PROPERTY "vsm_detail_force_include_${name}" ON
		)

		if(DEFINED VSM_OPT_COPY_FILE)
			file(
				COPY_FILE "${VSM_OPT_COPY_FILE}" "${force_include}"
				RESULT copy_file_result
			)

			if(NOT "${copy_file_result}" STREQUAL 0)
				message(SEND_ERROR "${copy_file_result}")
			endif()
		else()
			file(WRITE "${force_include}" "")
		endif()

		install(
			FILES "${force_include}"
			DESTINATION vsm_force_include
		)

		file(
			APPEND "${root_force_include}"
			"include(\"\${CMAKE_CURRENT_LIST_DIR}/vsm_force_include/${name}.cmake\")"
		)
	endif()
endfunction()

function(vsm_add_force_include name)
	vsm_detail_add_force_include(
		cmake
		"include(${name})\n"
	)
endfunction()
