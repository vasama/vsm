include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/detail/directory_test.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/detail/force_include.cmake")

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
	if ("${type}" STREQUAL "type-NOTFOUND")
		message(FATAL_ERROR "Target '${target}' was not defined using vsm_add_xxx")
	endif()
	set("${variable}" ${type} PARENT_SCOPE)
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

	if(DEFINED VSM_OPT_UNPARSED_ARGUMENTS)
		message(SEND_ERROR "vsm_configure: unrecognized arguments: ${VSM_OPT_UNPARSED_ARGUMENTS}")
	endif()
endmacro()

function(vsm_detail_add_file_set)
	cmake_parse_arguments(
		VSM_OPT2
		""
		"FILE_SET;TYPE;BASE_DIR;FILES;DESTINATION;CONFIGURE_VAR"
		""
		${ARGN}
	)

	if(do_configure)
		target_sources(
			"${target}"
			"${public_type}"
			FILE_SET "${VSM_OPT2_FILE_SET}"
			TYPE "${VSM_OPT2_TYPE}"
			BASE_DIRS "${VSM_OPT2_BASE_DIR}"
			FILES "${VSM_OPT2_FILES}"
		)

		get_target_property(
			"install"
			"${target}"
			"vsm_detail_install_${VSM_OPT2_FILE_SET}"
		)

		set(configure OFF)
		if("${install}" STREQUAL "install-NOTFOUND")
			set_property(
				TARGET "${target}"
				PROPERTY "vsm_detail_install_${VSM_OPT2_FILE_SET}" ""
			)

			set(destination "${VSM_OPT2_BASE_DIR}")
			if(DEFINED VSM_OPT2_DESTINATION)
				set(destination "${destination}/${VSM_OPT2_DESTINATION}")
			endif()

			install(
				TARGETS "${target}"
				FILE_SET "${VSM_OPT2_FILE_SET}"
				DESTINATION "${destination}"
			)

			set(configure ON)
		endif()

		if(DEFINED VSM_OPT2_CONFIGURE_VAR)
			set("${VSM_OPT2_CONFIGURE_VAR}" ${configure} PARENT_SCOPE)
		endif()
	endif()

	string(TOLOWER "${VSM_OPT2_FILE_SET}" directory_test_file_set)
	vsm_detail_add_directory_files("${directory_test_file_set}" "${VSM_OPT2_FILES}")
endfunction()

function(vsm_detail_configure target public_type)
	vsm_detail_add_directory_test()

	if(DEFINED VSM_OPT_HEADERS)
		vsm_detail_add_file_set(
			FILE_SET "HEADERS"
			TYPE "HEADERS"
			BASE_DIR "include"
			FILES "${VSM_OPT_HEADERS}"
			#TODO: Revisit this: DESTINATION "${target}" # Install in subdirectory: include/target/...
		)
	endif()

	if(DEFINED VSM_OPT_SOURCES)
		if(do_configure)
			target_sources(${target} PRIVATE ${VSM_OPT_SOURCES})
		endif()

		vsm_detail_add_directory_files("sources" "${VSM_OPT_SOURCES}")
	endif()

	if(DEFINED VSM_OPT_VISUALIZERS)
		vsm_detail_add_file_set(
			FILE_SET "visualizers"
			TYPE "HEADERS"
			BASE_DIR "visualizers"
			FILES "${VSM_OPT_VISUALIZERS}"
			CONFIGURE_VAR configure_visualizers
		)

		if(${configure_visualizers})
#			set(visualizers_module "${PROJECT_BINARY_DIR}/vsm_cmake_build_modules/import_visualizers.cmake")
#
#			get_property(
#				has_visualizers_module
#				DIRECTORY "${PROJECT_BINARY_DIR}"
#				PROPERTY vsm_detail_visualizers_module SET
#			)
#
#			if(NOT ${has_visualizers_module})
#				set_property(
#					DIRECTORY "${PROJECT_BINARY_DIR}"
#					PROPERTY vsm_detail_visualizers_module ON
#				)
#
#				file(WRITE "${visualizers_module}" "")
#
#				install(
#					FILES "${visualizers_module}"
#					DESTINATION "cmake_build_modules"
#				)
#			endif()
#
			set(visualizers "${VSM_OPT_VISUALIZERS}")
			#list(TRANSFORM visualizers PREPEND "\${CMAKE_CURRENT_LIST_DIR}/../")
			list(JOIN visualizers " " visualizers)
#
#			file(
#				APPEND "${visualizers_module}"
#				"TARGET ${name} VISUALIZERS ${visualizers}\n"
#			)

#			vsm_detail_add_cmake_build_module(
#				vsm_import_visualizers
#				COPY_FILE "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/detail/import_visualizers.cmake"
#				"vsm_detail_import_visualizers(${name} ${visualizers})\n"
#			)

			vsm_detail_add_force_include(
				import_visualizers
				COPY_FILE "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/detail/import_visualizers.cmake"
				"vsm_detail_import_visualizers(${name} ${visualizers})\n"
			)
		endif()
	endif()

	if(do_configure AND DEFINED VSM_OPT_HEADER_DEPENDENCIES)
		target_link_libraries(${target} ${public_type} ${VSM_OPT_HEADER_DEPENDENCIES})
	endif()

	if(do_configure AND DEFINED VSM_OPT_SOURCE_DEPENDENCIES)
		target_link_libraries(${target} PRIVATE ${VSM_OPT_SOURCE_DEPENDENCIES})
	endif()

	if(do_configure AND DEFINED VSM_OPT_HEADER_DEFINITIONS)
		target_compile_definitions(${target} ${public_type} ${VSM_OPT_HEADER_DEFINITIONS})
	endif()

	if(do_configure AND DEFINED VSM_OPT_SOURCE_DEFINITIONS)
		target_compile_definitions(${target} PRIVATE ${VSM_OPT_SOURCE_DEFINITIONS})
	endif()

	if(do_configure)
		if(DEFINED VSM_OPT_TEST_SOURCES)
			get_target_property(test_target ${target} vsm_detail_test_target)

			if(${test_target} STREQUAL "test_target-NOTFOUND")
				set(test_target ${target}_test)
				set_property(TARGET ${target} PROPERTY vsm_detail_test_target ${test_target})

				get_property(
					test_suite_target
					DIRECTORY "${PROJECT_SOURCE_DIR}"
					PROPERTY vsm_detail_test_suite_target
				)

				if("${test_suite_target}" STREQUAL "")
					add_executable(${test_target})
					add_test(${target} ${test_target})
					set_target_properties(${test_target} PROPERTIES FOLDER "Tests")
				else()
					add_library(${test_target} OBJECT)
					target_link_libraries(${test_suite_target} PRIVATE ${test_target})
					set_target_properties(${test_target} PROPERTIES FOLDER "TestComponents")
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

	if(do_configure AND DEFINED VSM_OPT_PROPERTIES)
		set_target_properties(
			${target}
			PROPERTIES ${VSM_OPT_PROPERTIES}
		)
	endif()

	if(do_configure AND DEFINED VSM_OPT_FOLDER)
		set_target_properties(
			${target}
			PROPERTIES FOLDER "${VSM_OPT_FOLDER}"
		)
	endif()
endfunction()

function(vsm_detail_add_target function name)
	string(REPLACE "::" "_" target ${name})
	string(REPLACE "-" "_" target ${target})

	cmake_language(CALL ${function} ${target} ${ARGN})

	if(NOT "x${target}" STREQUAL "x${name}")
		cmake_language(CALL ${function} ${name} ALIAS ${target})
	endif()

	set(target ${target} PARENT_SCOPE)
	set(do_configure ON PARENT_SCOPE)
endfunction()

function(vsm_add_executable name)
	vsm_detail_configure_parse(
		exe
		""
		""
		""
		${ARGN}
	)

	vsm_detail_add_target(add_executable ${name})

	set_target_properties(${target} PROPERTIES vsm_detail_type exe)
	target_include_directories(${target} PRIVATE source)
	target_link_libraries(${target} PRIVATE vsm_detail_cxx_options)

	if(DEFINED VSM_EXPORT_COMPONENTS)
		file(APPEND "${VSM_EXPORT_COMPONENTS}" "${name}\n")
	endif()

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

	set(add_library_args "")
	set(public_type INTERFACE)
	set(private_type INTERFACE)

	if(DEFINED VSM_OPT_SOURCES OR VSM_OPT_ADDITIONAL_SOURCES)
		set(public_type PUBLIC)
		set(private_type PRIVATE)
	else()
		list(APPEND add_library_args "INTERFACE")
	endif()

	vsm_detail_add_target(add_library ${name} ${add_library_args})

	set_target_properties(${target} PROPERTIES vsm_detail_type lib)
	target_include_directories(${target} ${public_type} include)

	if(${private_type} STREQUAL PRIVATE)
		target_include_directories(${target} PRIVATE source)
		target_link_libraries(${target} PRIVATE vsm_detail_cxx_options)
	endif()

	vsm_detail_configure(${target} ${public_type})
endfunction()

function(vsm_configure name)
	get_target_property(target_type ${name} vsm_detail_type)
	if ("${target_type}" STREQUAL "type-NOTFOUND")
		message(SEND_ERROR "Target '${target}' was not defined using vsm_add_xxx")
		return()
	endif()

	vsm_detail_configure_parse(
		"${target_type}"
		""
		"PLATFORM"
		""
		${ARGN}
	)

	string(REPLACE "::" "_" target ${name})
	string(REPLACE "-" "_" target ${target})

	get_target_property(public_type ${target} TYPE)
	if (${public_type} STREQUAL INTERFACE_LIBRARY)
		set(public_type INTERFACE)
	else()
		set(public_type PUBLIC)
	endif()

	set(do_configure ON)
	if(DEFINED VSM_OPT_PLATFORM)
		if(NOT "x${CMAKE_SYSTEM_NAME}" STREQUAL "x${VSM_OPT_PLATFORM}")
			set(do_configure OFF)
		endif()
	endif()

	vsm_detail_configure(${target} ${public_type})
endfunction()
