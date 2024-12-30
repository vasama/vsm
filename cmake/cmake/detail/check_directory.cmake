function(vsm_detail_check_directory root_dir source_dir binary_dir)
	set(DIRTEST_ROOT_DIR "${root_dir}")
	set(DIRTEST_SOURCE_DIR "${source_dir}")
	set(DIRTEST_BINARY_DIR "${binary_dir}")
	include("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scripts/check_directory.cmake")
endfunction()

function(vsm_detail_add_directory_test)
	get_property(directory_test_set DIRECTORY PROPERTY "vsm_detail_check_directory" SET)

	if(NOT ${directory_test_set})
		file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/vsm_detail_check_directory")
		set_property(DIRECTORY PROPERTY "vsm_detail_check_directory" ON)

		if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.19)
			cmake_language(
				EVAL CODE
				"
				cmake_language(
					DEFER DIRECTORY \"${CMAKE_SOURCE_DIR}\"
					CALL
					vsm_detail_check_directory
						\"${CMAKE_SOURCE_DIR}\"
						\"${CMAKE_CURRENT_SOURCE_DIR}\"
						\"${CMAKE_CURRENT_BINARY_DIR}\"
				)
				"
			)
		else()
			cmake_path(
				RELATIVE_PATH CMAKE_CURRENT_SOURCE_DIR
				BASE_DIRECTORY "${CMAKE_SOURCE_DIR}"
				OUTPUT_VARIABLE relative_current_source_dir)

			if("${relative_current_source_dir}" STREQUAL "")
				set(relative_current_source_dir "<root directory>")
			endif()

			add_test(
				NAME "configuration: ${relative_current_source_dir}"
				COMMAND
					"${CMAKE_COMMAND}"
						-Wauthor
						-Werror=author
						-D "DIRTEST_ROOT_DIR=${CMAKE_SOURCE_DIR}"
						-D "DIRTEST_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
						-D "DIRTEST_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}"
						-P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/scripts/check_directory.cmake"
			)
		endif()
	endif()
endfunction()

function(vsm_detail_add_directory_files file_set files)
	vsm_detail_add_directory_test()

	list(JOIN files "\n" lines)
	file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/vsm_detail_check_directory/${file_set}.txt" "${lines}\n")
endfunction()

function(vsm_ignore_files)
	vsm_detail_add_directory_files("ignored" "${ARGN}")
endfunction()
