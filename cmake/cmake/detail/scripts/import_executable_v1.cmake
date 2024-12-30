if(NOT COMMAND vsm_detail_import_executable_v1)
	function(vsm_detail_import_executable_v1 target executable_name)
		find_program(executable_path "${executable_name}" REQUIRED)
		add_executable("${target}" IMPORTED)
		set_target_properties(
			"${target}"
			PROPERTIES IMPORTED_LOCATION "${executable_path}"
		)
	endfunction()
endif()
