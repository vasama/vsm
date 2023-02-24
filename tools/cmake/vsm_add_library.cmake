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

	set(public_type INTERFACE)

	if(DEFINED OPT_SOURCES OR OPT_ADDITIONAL_SOURCES)
		set(public_type PUBLIC)
	endif()

	set(add_library_args "")

	if(public_type STREQUAL "INTERFACE")
		list(APPEND add_library_args "INTERFACE")
	endif()

	add_library(${target} ${add_library_args})

	target_sources(${target}
		${public_type}
		FILE_SET HEADERS
		BASE_DIRS include
		FILES ${OPT_HEADERS}
	)

	target_include_directories(${target} ${public_type} include)

	if(DEFINED OPT_SOURCES)
		target_sources(${target} PRIVATE ${OPT_SOURCES})
		target_include_directories(${target} PRIVATE source)
	endif()

	if(DEFINED HEADER_DEPENDENCIES)
		target_link_libraries(${target} ${public_type} ${OPT_DEPENDENCIES})
	endif()

	if(DEFINED SOURCE_DEPENDENCIES)
		target_link_libraries(${target} PRIVATE ${OPT_DEPENDENCIES})
	endif()

	install(TARGETS ${target} FILE_SET HEADERS)

	if(DEFINED OPT_EXPORT_ALIAS AND NOT PROJECT_IS_TOP_LEVEL)
		add_library(${OPT_EXPORT_ALIAS} ALIAS ${target})
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
