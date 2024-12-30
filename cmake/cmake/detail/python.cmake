include_guard(GLOBAL)

function(vsm_detail_setup_python python_var)
	set(requirements "${ARGN}")

	find_package(Python REQUIRED)
	set(python "${Python_EXECUTABLE}")

	if(NOT "${requirements}" STREQUAL "")
		set(venv_path "${CMAKE_BINARY_DIR}/vsm_python_venv")
		if(NOT EXISTS "${venv_path}")
			set(temp_path "${venv_path}.tmp")
			if(EXISTS "${temp_path}")
				file(REMOVE_RECURSE "${temp_path}")
			endif()

			# Create virtual environment in temporary directory:
			execute_process(
				COMMAND_ERROR_IS_FATAL ANY
				COMMAND "${python}" -m venv "${temp_path}"
			)

			# Rename temp directory to finalize creation:
			file(RENAME "${temp_path}" "${venv_path}")
		endif()

		if(CMAKE_HOST_WIN32)
			set(python "${venv_path}/Scripts/python.exe")
		else()
			set(python "${venv_path}/bin/python")
		endif()

		# Install wheels in the Python virtual environment:
		execute_process(
			COMMAND_ERROR_IS_FATAL ANY
			COMMAND "${python}" -m pip install ${requirements}
		)
	endif()

	# Set the python path in the parent scope.
	set("${python_var}" "${python}" PARENT_SCOPE)
endfunction()

function(vsm_add_python_target target)
	cmake_parse_arguments(
		OPT
		""
		""
		"REQUIREMENTS;DEPENDS;OUTPUTS;COMMAND"
		$ARGN
	)

	if(DEFINED OPT_UNPARSED_ARGUMENTS)
		message(SEND_ERROR "vsm_add_python_target: unrecognized arguments: ${OPT_UNPARSED_ARGUMENTS}")
	endif()

	vsm_detail_setup_python(
		python
		REQUIREMENTS ${OPT_REQUIREMENTS}
	)

	add_custom_command(
		DEPENDS ${OPT_DEPENDS}
		OUTPUT ${OPT_OUTPUTS}
		COMMAND "${python}" -c ${OPT_COMMAND}
	)

	add_custom_target(
		"${target}"
		DEPENDS ${OPT_OUTPUT}
	)
endfunction()
