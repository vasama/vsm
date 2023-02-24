function(vsm_detail_add_test_suite test_suite_target)
	add_executable("${test_suite_target}")
	add_test("${test_suite_target}" "${test_suite_target}")

	define_property(DIRECTORY PROPERTY vsm_detail_test_suite_target INHERITED)
	set_property(DIRECTORY PROPERTY vsm_detail_test_suite_target "${test_suite_target}")
	
	# Set the Visual Studio debugger working directory for the target:
	set_property(
		TARGET "${test_suite_target}"
		PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	)

	# Make the aggregate test target the default Visual Studio startup project:
	get_property(
		has_startup_property
		DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		PROPERTY VS_STARTUP_PROJECT SET
	)
	if(NOT ${has_startup_property})
		set_property(
			DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
			PROPERTY VS_STARTUP_PROJECT "${test_suite_target}")
	endif()
endfunction()

function(vsm_add_root name)
	cmake_parse_arguments(
		VSM_OPT
		"NO_TEST_SUITE"
		""
		""
		${ARGN}
	)

	if(NOT "${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}")
		message(FATAL_ERROR "vsm_root must be called in the project CMakeLists.txt")
	endif()

	get_property(has_root_name_property DIRECTORY PROPERTY vsm_detail_root_name DEFINED)
	if(${has_root_name_property})
		message(FATAL_ERROR "vsm_root has already been called in this directory.")
	endif()
	define_property(DIRECTORY PROPERTY vsm_detail_root_name INHERITED)
	set_property(DIRECTORY PROPERTY vsm_detail_root_name "${name}")

	if(NOT ${VSM_OPT_NO_TEST_SUITE})
		# Define the test suite target:
		vsm_detail_add_test_suite("${name}-test-suite")
	endif()
endfunction()
