function(vsm_add_library target)
	set(options
		ADDITIONAL_SOURCES
	)

	set(one_value_keywords
		EXPORT_ALIAS
	)

	set(multi_value_keywords
		HEADERS
		SOURCES
		HEADER_DEPENDENCIES
		SOURCE_DEPENDENCIES
		TEST_SOURCES
		TEST_DEPENDENCIES
	)

	cmake_parse_arguments(OPT "${options}" "${one_value_keywords}" "${multi_value_keywords}" ${ARGN})

	set(add_library_args "")
	set(public_type INTERFACE)
	set(private_type INTERFACE)

	if(DEFINED OPT_SOURCES OR OPT_ADDITIONAL_SOURCES)
		set(public_type PUBLIC)
		set(private_type PRIVATE)
	else()
		list(APPEND add_library_args "INTERFACE")
	endif()

	add_library(${target} ${add_library_args})

	target_include_directories(${target} ${public_type} include)
	target_sources(${target}
		${public_type}
		FILE_SET HEADERS
		BASE_DIRS include
		FILES ${OPT_HEADERS}
	)

	if(DEFINED OPT_SOURCES)
		target_include_directories(${target} PRIVATE source)
		target_sources(${target} PRIVATE ${OPT_SOURCES})
	endif()

	if(DEFINED OPT_HEADER_DEPENDENCIES)
		target_link_libraries(${target} ${public_type} ${OPT_HEADER_DEPENDENCIES})
	endif()

	if(DEFINED OPT_SOURCE_DEPENDENCIES)
		target_link_libraries(${target} PRIVATE ${OPT_SOURCE_DEPENDENCIES})
	endif()

	target_compile_options(${target} ${private_type} ${vsm_compile_options})

	install(TARGETS ${target} FILE_SET HEADERS)

	if(DEFINED OPT_EXPORT_ALIAS AND NOT PROJECT_IS_TOP_LEVEL)
		add_library(${OPT_EXPORT_ALIAS} ALIAS ${target})
	endif()


	if(DEFINED OPT_TEST_SOURCES) # Unit test
		set(test_target ${target}_test)

		if(DEFINED vsm_aggregate_test_target)
			add_library(${test_target} OBJECT)
			target_link_libraries(${vsm_aggregate_test_target} PRIVATE ${test_target})
		else()
			add_executable(${test_target})
			add_test(${target} ${test_target})
		endif()

		target_sources(${test_target} PRIVATE ${OPT_TEST_SOURCES})
		target_include_directories(${test_target} PRIVATE source)

		target_link_libraries(${test_target} PRIVATE Catch2::Catch2WithMain)
		target_link_libraries(${test_target} PRIVATE ${target})

		if(DEFINED OPT_TEST_DEPENDENCIES)
			target_link_libraries(${test_target} PRIVATE ${OPT_TEST_DEPENDENCIES})
		endif()

		target_compile_options(${test_target} PRIVATE ${vsm_compile_options})
	endif()
endfunction()

function(vsm_target_platform_sources target platform)
	if(${CMAKE_SYSTEM_NAME} STREQUAL "${platform}")
		target_sources(${target} PRIVATE ${ARGN})
	endif()
endfunction()

function(vsm_target_platform_dependencies target platform)
	if(${CMAKE_SYSTEM_NAME} STREQUAL "${platform}")
		target_link_libraries(${target} PRIVATE ${ARGN})
	endif()
endfunction()

function(vsm_target_platform target platform)
	if(${CMAKE_SYSTEM_NAME} STREQUAL "${platform}")
	endif()
endfunction()
