function(vsm_detail_configure target public_type private_type)
	if(DEFINED OPT_HEADERS)
		target_sources(${target}
			${public_type}
			FILE_SET HEADERS
			BASE_DIRS include
			FILES ${OPT_HEADERS}
		)

		get_target_property(install_headers ${target} vsm_detail_install_headers)
		if("${install_headers}" STREQUAL "install_headers-NOTFOUND")
			set_property(TARGET ${target} PROPERTY vsm_detail_install_headers "")
			install(TARGETS ${target} FILE_SET HEADERS)
		endif()
	endif()

	if(DEFINED OPT_SOURCES)
		target_sources(${target} PRIVATE ${OPT_SOURCES})
	endif()

	if(DEFINED OPT_HEADER_DEPENDENCIES)
		target_link_libraries(${target} ${public_type} ${OPT_HEADER_DEPENDENCIES})
	endif()

	if(DEFINED OPT_SOURCE_DEPENDENCIES)
		target_link_libraries(${target} PRIVATE ${OPT_SOURCE_DEPENDENCIES})
	endif()

	if(DEFINED OPT_TEST_SOURCES OR DEFINED OPT_TEST_DEPENDENCIES)
		get_target_property(test_target ${target} vsm_detail_test_target)

		if(${test_target} STREQUAL "test_target-NOTFOUND")
			set(test_target ${target}_test)
			set_property(TARGET ${target} PROPERTY vsm_detail_test_target ${test_target})

			if(DEFINED vsm_aggregate_test_target)
				add_library(${test_target} OBJECT)
				target_link_libraries(${vsm_aggregate_test_target} PRIVATE ${test_target})
			else()
				add_executable(${test_target})
				add_test(${target} ${test_target})
			endif()
			
			target_include_directories(${test_target} PRIVATE source)
			target_compile_options(${test_target} PRIVATE ${vsm_compile_options})
			
			target_link_libraries(${test_target} PRIVATE ${target})
			target_link_libraries(${test_target} PRIVATE Catch2::Catch2WithMain)
		endif()

		if(DEFINED OPT_TEST_SOURCES)
			target_sources(${test_target} PRIVATE ${OPT_TEST_SOURCES})
		endif()

		if(DEFINED OPT_TEST_DEPENDENCIES)
			target_link_libraries(${test_target} PRIVATE ${OPT_TEST_DEPENDENCIES})
		endif()
	endif()
endfunction()

function(vsm_add_library name)
	set(options
		ADDITIONAL_SOURCES
	)
	set(multi_value_keywords
		HEADERS
		SOURCES
		HEADER_DEPENDENCIES
		SOURCE_DEPENDENCIES
		TEST_SOURCES
		TEST_DEPENDENCIES
	)
	cmake_parse_arguments(OPT "${options}" "" "${multi_value_keywords}" ${ARGN})

	string(REPLACE "::" "_" target ${name})

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
	
	if(NOT ${target} STREQUAL ${name})
		add_library(${name} ALIAS ${target})
	endif()

	target_include_directories(${target} ${public_type} include)

	if(${public_type} STREQUAL PUBLIC)
		target_include_directories(${target} PRIVATE source)
	endif()

	target_compile_options(${target} ${private_type} ${vsm_compile_options})

	vsm_detail_configure(${target} ${public_type} ${private_type})
endfunction()

function(vsm_configure name)
	set(one_value_keywords
		PLATFORM
	)
	set(multi_value_keywords
		HEADERS
		SOURCES
		HEADER_DEPENDENCIES
		SOURCE_DEPENDENCIES
		TEST_SOURCES
		TEST_DEPENDENCIES
	)
	cmake_parse_arguments(OPT "" "${one_value_keywords}" "${multi_value_keywords}" ${ARGN})

	if(DEFINED OPT_PLATFORM)
		if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL ${OPT_PLATFORM})
			return()
		endif()
	endif()

	string(REPLACE "::" "_" target ${name})

	set(public_type PUBLIC)
	set(private_type PRIVATE)

	get_target_property(target_type ${target} TYPE)
	if (target_type STREQUAL INTERFACE_LIBRARY)
		set(public_type INTERFACE)
		set(private_type INTERFACE)
	endif()

	vsm_detail_configure(${target} ${public_type} ${private_type})
endfunction()
