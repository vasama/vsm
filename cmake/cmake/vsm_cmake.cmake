include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/detail/directory_test.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/detail/force_include.cmake")

function(vsm_add_cmake)
	cmake_parse_arguments(
		VSM_OPT
		"FORCE_INCLUDE"
		""
		""
		${ARGN}
	)

	install(
		FILES ${VSM_OPT_UNPARSED_ARGUMENTS}
		DESTINATION cmake
	)

	vsm_detail_add_directory_files(
		cmake
		${VSM_OPT_UNPARSED_ARGUMENTS}
	)

	if(${VSM_OPT_FORCE_INCLUDE})
		set(force_include_content)

		foreach(cmake_file ${VSM_OPT_UNPARSED_ARGUMENTS})
			cmake_path(GET cmake_file PARENT_PATH cmake_file_parent_path)

			if(NOT "${cmake_file_parent_path}" STREQUAL "cmake")
				message(SEND_ERROR "Script must be nested directly inside ./cmake/")
			endif()
	
			if(${VSM_OPT_FORCE_INCLUDE})
				cmake_path(GET cmake_file STEM cmake_file_stem)
				list(APPEND force_includes "include(${cmake_file_stem})\n")
			endif()
		endforeach()

		vsm_detail_add_force_include(
			cmake
			${force_include_content}
		)
	endif()
endfunction()
