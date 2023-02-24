function(vsm_add_test target)
	set(test_target "${target}_test")
		
	if(DEFINED vsm_test_aggregate_target)
		set(test_target vsm_test_aggregate_target)
	else()
		add_executable(${test_target})
		target_link_libraries(${test_target}
			PRIVATE Catch2::CatchWithMain
		)
		add_test(${test_target} ${test_target})
	endif()
	
	
endfunction()
