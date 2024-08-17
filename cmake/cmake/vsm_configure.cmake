set(vsm_detail_configure_opt_1
	FOLDER
)
set(vsm_detail_configure_opt_n
	SOURCES
	SOURCE_DEPENDENCIES
	SOURCE_DEFINITIONS
	PROPERTIES
	VISUALIZERS
)
set(vsm_detail_configure_lib_opt_n
	HEADERS
	HEADER_DEPENDENCIES
	HEADER_DEFINITIONS
	TEST_SOURCES
	TEST_DEPENDENCIES
)

function(vsm_detail_get_target_type target variable)
	get_target_property(type ${target} TYPE)
	if(${type} STREQUAL EXECUTABLE)
		set(${variable} "exe" PARENT_SCOPE)
	elseif(${type} MATCHES "[A-Z]+_LIBRARY")
		set(${variable} "lib" PARENT_SCOPE)
	else()
		message(SEND_ERROR "Cannot determine the type of target ${target} (${type})")
	endif()
endfunction()

macro(vsm_detail_configure_parse type opt_0 opt_1 opt_n)
	set(vsm_detail_opt_0 ${opt_0})
	set(vsm_detail_opt_1 ${opt_1})
	set(vsm_detail_opt_n ${opt_n})

	list(APPEND vsm_detail_opt_1 ${vsm_detail_configure_opt_1})
	list(APPEND vsm_detail_opt_n ${vsm_detail_configure_opt_n})

	if(DEFINED vsm_detail_configure_${type}_opt_0)
		list(APPEND vsm_detail_opt_0 ${vsm_detail_configure_${type}_opt_0})
	endif()
	if(DEFINED vsm_detail_configure_${type}_opt_1)
		list(APPEND vsm_detail_opt_1 ${vsm_detail_configure_${type}_opt_1})
	endif()
	if(DEFINED vsm_detail_configure_${type}_opt_n)
		list(APPEND vsm_detail_opt_n ${vsm_detail_configure_${type}_opt_n})
	endif()

	cmake_parse_arguments(
		VSM_OPT
		"${vsm_detail_opt_0}"
		"${vsm_detail_opt_1}"
		"${vsm_detail_opt_n}"
		${ARGN}
	)

	#TODO: Figure out why this isn't firing
	if(DEFINED MY_INSTALL_UNPARSED_ARGUMENTS)
		message(ERROR "vsm_configure: unrecognized arguments: ${MY_INSTALL_UNPARSED_ARGUMENTS}")
	endif()
endmacro()

function(vsm_detail_add_file_set target public file_set type base_dir files)
	target_sources(
		"${target}"
		"${public}"
		FILE_SET "${file_set}"
		TYPE "${type}"
		BASE_DIRS "${base_dir}"
		FILES "${files}"
	)

	get_target_property(
		"install"
		"${target}"
		"vsm_detail_install_${file_set}"
	)

	if("${install}" STREQUAL "install-NOTFOUND")
		set_property(
			TARGET "${target}"
			PROPERTY "vsm_detail_install_${file_set}" ""
		)
		install(
			TARGETS "${target}"
			FILE_SET "${file_set}"
		)
	endif()
endfunction()

function(vsm_detail_configure target public_type)
	if(DEFINED VSM_OPT_HEADERS)
		vsm_detail_add_file_set(
			"${target}"
			"${public_type}"
			"HEADERS"
			"HEADERS"
			"include"
			"${VSM_OPT_HEADERS}"
		)

#		target_sources(${target}
#			${public_type}
#			FILE_SET HEADERS
#			BASE_DIRS include
#			FILES ${VSM_OPT_HEADERS}
#		)
#
#		get_target_property(install_headers ${target} vsm_detail_install_headers)
#		if("${install_headers}" STREQUAL "install_headers-NOTFOUND")
#			set_property(TARGET ${target} PROPERTY vsm_detail_install_headers "")
#			install(TARGETS ${target} FILE_SET HEADERS)
#		endif()
	endif()

	if(DEFINED VSM_OPT_SOURCES)
		target_sources(${target} PRIVATE ${VSM_OPT_SOURCES})
	endif()

	if(DEFINED VSM_OPT_VISUALIZERS)
		vsm_detail_add_file_set(
			"${target}"
			"${public_type}"
			"visualizers"
			"HEADERS"
			"visualizers"
			"${VSM_OPT_VISUALIZERS}"
		)
	endif()

	if(DEFINED VSM_OPT_HEADER_DEPENDENCIES)
		target_link_libraries(${target} ${public_type} ${VSM_OPT_HEADER_DEPENDENCIES})
	endif()

	if(DEFINED VSM_OPT_SOURCE_DEPENDENCIES)
		target_link_libraries(${target} PRIVATE ${VSM_OPT_SOURCE_DEPENDENCIES})
	endif()

	if(DEFINED VSM_OPT_HEADER_DEFINITIONS)
		target_compile_definitions(${target} ${public_type} ${VSM_OPT_HEADER_DEFINITIONS})
	endif()

	if(DEFINED VSM_OPT_SOURCE_DEFINITIONS)
		target_compile_definitions(${target} PRIVATE ${VSM_OPT_SOURCE_DEFINITIONS})
	endif()

	if(DEFINED VSM_OPT_TEST_SOURCES)
		get_target_property(test_target ${target} vsm_detail_test_target)

		if(${test_target} STREQUAL "test_target-NOTFOUND")
			set(test_target ${target}_test)
			set_property(TARGET ${target} PROPERTY vsm_detail_test_target ${test_target})

			if(DEFINED vsm_aggregate_test_target)
				add_library(${test_target} OBJECT)
				target_link_libraries(${vsm_aggregate_test_target} PRIVATE ${test_target})
				set_target_properties(${test_target} PROPERTIES FOLDER "TestLibraries")
			else()
				add_executable(${test_target})
				add_test(${target} ${test_target})
				set_target_properties(${test_target} PROPERTIES FOLDER "Tests")
			endif()

			target_include_directories(${test_target} PRIVATE source)

			target_link_libraries(
				${test_target}

				PRIVATE
					${target}
					vsm_cmake_options
					Catch2::Catch2WithMain
			)
		endif()

		target_sources(${test_target} PRIVATE ${VSM_OPT_TEST_SOURCES})

		if(DEFINED VSM_OPT_TEST_DEPENDENCIES)
			target_link_libraries(${test_target} PRIVATE ${VSM_OPT_TEST_DEPENDENCIES})
		endif()
	else()
		if(DEFINED VSM_OPT_TEST_DEPENDENCIES)
			message(SEND_ERROR "Cannot specify test dependencies for target ${target} without test sources.")
		endif()
	endif()

	if(DEFINED VSM_OPT_PROPERTIES)
		set_target_properties(
			${target}
			PROPERTIES ${VSM_OPT_PROPERTIES}
		)
	endif()

	if(DEFINED VSM_OPT_FOLDER)
		set_target_properties(
			${target}
			PROPERTIES FOLDER "${VSM_OPT_FOLDER}"
		)
	endif()

	if(DEFINED VSM_OPT_VISUALIZERS)
		target_sources(
			${target}
			${public_type}
			${VSM_OPT_VISUALIZERS}
		)
	endif()
endfunction()

function(vsm_add_executable name)
	vsm_detail_configure_parse(exe "" "" "" ${ARGN})

	string(REPLACE "::" "_" target ${name})

	add_executable(${target})

	if(NOT "x${target}" STREQUAL "x${name}")
		add_executable("${name}" ALIAS "${target}")
	endif()

	target_include_directories(${target} PRIVATE source)
	target_link_libraries(${target} PRIVATE vsm_cmake_options)

	vsm_detail_configure(${target} PUBLIC)
endfunction()

function(vsm_add_library name)
	vsm_detail_configure_parse(
		lib
		"ADDITIONAL_SOURCES"
		""
		""
		${ARGN}
	)

	string(REPLACE "::" "_" target ${name})

	set(add_library_args "")
	set(public_type INTERFACE)
	set(private_type INTERFACE)

	if(DEFINED VSM_OPT_SOURCES OR VSM_OPT_ADDITIONAL_SOURCES)
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

	if(${private_type} STREQUAL PRIVATE)
		target_include_directories(${target} PRIVATE source)
		target_link_libraries(${target} PRIVATE vsm_cmake_options)
	endif()

	vsm_detail_configure(${target} ${public_type})
endfunction()

function(vsm_configure name)
	vsm_detail_get_target_type(${name} target_type)

	vsm_detail_configure_parse(
		"${target_type}"
		""
		"PLATFORM"
		""
		${ARGN}
	)

	#cmake_parse_arguments(
	#	VSM_OPT
	#	"PLATFORM;${vsm_detail_configure_opt_1}"
	#	"${vsm_detail_configure_lib_opt_n}"
	#	${ARGN}
	#)
	#set(one_value_keywords
	#	PLATFORM
	#)
	#set(multi_value_keywords
	#	HEADERS
	#	SOURCES
	#	HEADER_DEPENDENCIES
	#	SOURCE_DEPENDENCIES
	#	HEADER_DEFINITIONS
	#	SOURCE_DEFINITIONS
	#	TEST_SOURCES
	#	TEST_DEPENDENCIES
	#	FOLDER
	#	VISUALIZERS
	#)
	#cmake_parse_arguments(VSM_OPT "" "${one_value_keywords}" "${multi_value_keywords}" ${ARGN})

	if(DEFINED VSM_OPT_PLATFORM)
		if(NOT "x${CMAKE_SYSTEM_NAME}" STREQUAL "x${VSM_OPT_PLATFORM}")
			return()
		endif()
	endif()

	string(REPLACE "::" "_" target ${name})

	set(public_type PUBLIC)
	set(private_type PRIVATE)

	get_target_property(target_type ${target} TYPE)
	if (${target_type} STREQUAL INTERFACE_LIBRARY)
		set(public_type INTERFACE)
		set(private_type INTERFACE)
	endif()

	vsm_detail_configure(${target} ${public_type})
endfunction()
