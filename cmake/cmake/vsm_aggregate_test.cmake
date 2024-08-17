function(vsm_add_aggregate_test target)
	if(DEFINED vsm_aggregate_test_target)
		message(SEND_ERROR "aggregate test target already defined")
		return()
	endif()

	add_executable(${target})
	set(vsm_aggregate_test_target ${target} PARENT_SCOPE)

	set_property(
		TARGET ${target}
		PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

	# Make the aggregate test target the default Visual Studio startup project.
	get_property(startup_project DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT)
	if("x${startup_project}" STREQUAL "x")
		set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT "${target}")
	endif()
endfunction()
