include_guard(GLOBAL)

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

function(vsm_detail_add_directory_test)
	get_property(directory_test_set DIRECTORY PROPERTY "vsm_detail_directory_test" SET)

	if(NOT ${directory_test_set})
		cmake_path(
			RELATIVE_PATH CMAKE_CURRENT_SOURCE_DIR
			BASE_DIRECTORY PROJECT_SOURCE_DIR
			OUTPUT_VARIABLE relative_current_source_dir)

		add_test(
			NAME "${relative_current_source_dir} configuration"
			COMMAND
				cmake
					-D "DIRTEST_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
					-D "DIRTEST_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}"
					-P "${vsm_detail_cmake_root}/detail/directory_test.cmake"
		)

		file(REMOVE_RECURSE "${CMAKE_CURRENT_BINARY_DIR}/vsm_detail_directory_test")

		set_property(DIRECTORY PROPERTY "vsm_detail_directory_test" ON)
	endif()
endfunction()

function(vsm_detail_add_directory_files file_set files)
	list(JOIN files "\n" lines)
	file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/vsm_detail_directory_test/${file_set}.txt" "${lines}\n")
endfunction()

function(vsm_detail_add_file_set target public file_set type base_dir files)
	if(VSM_DO_CONFIGURE)
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
				DESTINATION "${base_dir}"
			)
		endif()
	endif()

	string(TOLOWER "${file_set}" directory_test_file_set)
	vsm_detail_add_directory_files("${directory_test_file_set}" "${files}")
endfunction()

function(vsm_detail_configure target public_type)
	vsm_detail_add_directory_test()

	if(DEFINED VSM_OPT_HEADERS)
		vsm_detail_add_file_set(
			"${target}"
			"${public_type}"
			"HEADERS"
			"HEADERS"
			"include"
			"${VSM_OPT_HEADERS}"
		)
	endif()

	if(DEFINED VSM_OPT_SOURCES)
		if(VSM_DO_CONFIGURE)
			target_sources(${target} PRIVATE ${VSM_OPT_SOURCES})
		endif()

		vsm_detail_add_directory_files("sources" "${VSM_OPT_SOURCES}")
	endif()

	if(DEFINED VSM_OPT_VISUALIZERS)
		get_target_property(
			"install"
			"${target}"
			"vsm_detail_install_${file_set}"
		)

		vsm_detail_add_file_set(
			"${target}"
			"${public_type}"
			"visualizers"
			"HEADERS"
			"visualizers"
			"${VSM_OPT_VISUALIZERS}"
		)

		if(VSM_DO_CONFIGURE AND "${install}" STREQUAL "install-NOTFOUND")
			install(
				FILES "${vsm_detail_cmake_root}/detail/configure_visualizers.cmake"
				DESTINATION "cmake_module")
		endif()
	endif()

	if(VSM_DO_CONFIGURE AND DEFINED VSM_OPT_HEADER_DEPENDENCIES)
		target_link_libraries(${target} ${public_type} ${VSM_OPT_HEADER_DEPENDENCIES})
	endif()

	if(VSM_DO_CONFIGURE AND DEFINED VSM_OPT_SOURCE_DEPENDENCIES)
		target_link_libraries(${target} PRIVATE ${VSM_OPT_SOURCE_DEPENDENCIES})
	endif()

	if(VSM_DO_CONFIGURE AND DEFINED VSM_OPT_HEADER_DEFINITIONS)
		target_compile_definitions(${target} ${public_type} ${VSM_OPT_HEADER_DEFINITIONS})
	endif()

	if(VSM_DO_CONFIGURE AND DEFINED VSM_OPT_SOURCE_DEFINITIONS)
		target_compile_definitions(${target} PRIVATE ${VSM_OPT_SOURCE_DEFINITIONS})
	endif()

	if(VSM_DO_CONFIGURE)
		if(DEFINED VSM_OPT_TEST_SOURCES)
			get_target_property(test_target ${target} vsm_detail_test_target)

			if(${test_target} STREQUAL "test_target-NOTFOUND")
				set(test_target ${target}_test)
				set_property(TARGET ${target} PROPERTY vsm_detail_test_target ${test_target})

				if(DEFINED vsm_aggregate_test_target)
					add_library(${test_target} OBJECT)
					target_link_libraries(${vsm_aggregate_test_target} PRIVATE ${test_target})
					set_target_properties(${test_target} PROPERTIES FOLDER "TestComponents")
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
						vsm_detail_cxx_options
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
	endif()

	if(VSM_DO_CONFIGURE AND DEFINED VSM_OPT_PROPERTIES)
		set_target_properties(
			${target}
			PROPERTIES ${VSM_OPT_PROPERTIES}
		)
	endif()

	if(VSM_DO_CONFIGURE AND DEFINED VSM_OPT_FOLDER)
		set_target_properties(
			${target}
			PROPERTIES FOLDER "${VSM_OPT_FOLDER}"
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
	target_link_libraries(${target} PRIVATE vsm_detail_cxx_options)

	set(VSM_DO_CONFIGURE ON)
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
		target_link_libraries(${target} PRIVATE vsm_detail_cxx_options)
	endif()

	set(VSM_DO_CONFIGURE ON)
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

	string(REPLACE "::" "_" target ${name})

	set(public_type PUBLIC)

	get_target_property(target_type ${target} TYPE)
	if (${target_type} STREQUAL INTERFACE_LIBRARY)
		set(public_type INTERFACE)
	endif()

	set(VSM_DO_CONFIGURE ON)

	if(DEFINED VSM_OPT_PLATFORM)
		if(NOT "x${CMAKE_SYSTEM_NAME}" STREQUAL "x${VSM_OPT_PLATFORM}")
			set(VSM_DO_CONFIGURE OFF)
		endif()
	endif()

	vsm_detail_configure(${target} ${public_type})
endfunction()
