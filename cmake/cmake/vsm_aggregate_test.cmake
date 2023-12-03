function(vsm_add_aggregate_test target)
	if(DEFINED vsm_aggregate_test_target)
		message(SEND_ERROR "aggregate test target already defined")
	else()
		add_executable(${target})
		set(vsm_aggregate_test_target ${target} PARENT_SCOPE)

		set_property(
			TARGET ${target}
			PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	endif()
endfunction()
