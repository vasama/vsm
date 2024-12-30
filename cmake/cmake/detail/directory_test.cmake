function(vsm_detail_add_directory_files file_set files)
	list(JOIN files "\n" lines)
	file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/vsm_detail_directory_test/${file_set}.txt" "${lines}\n")
endfunction()

function(vsm_detail_add_directory_test)
	get_property(directory_test_set DIRECTORY PROPERTY "vsm_detail_directory_test" SET)

	if(NOT ${directory_test_set})
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
				cmake
					-D "DIRTEST_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
					-D "DIRTEST_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}"
					-P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/directory_test_script.cmake"
		)

		file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/vsm_detail_directory_test")
		set_property(DIRECTORY PROPERTY "vsm_detail_directory_test" ON)
	endif()
endfunction()
