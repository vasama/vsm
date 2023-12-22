find_package(Catch2 REQUIRED)

add_library(vsm_cmake_options INTERFACE)

set(vsm_compiler_frontend ${CMAKE_CXX_COMPILER_ID})
if(DEFINED CMAKE_CXX_COMPILER_FRONTEND_VARIANT)
	set(vsm_compiler_frontend ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT})
endif()

if("${vsm_compiler_frontend}" STREQUAL "MSVC")
	target_compile_options(vsm_cmake_options INTERFACE
		/permissive-
		/Zc:preprocessor
		/Zc:__cplusplus

		/W4
		#TODO: Enable warnings as errors
		#/WX

		# Disable C4324: Structure was padded due to alignment specifier
		/wd4324
	)

	target_compile_definitions(vsm_cmake_options INTERFACE
		# Disable Microsoft specific C stdlib deprecation warnings
		_CRT_SECURE_NO_WARNINGS
		# Disable Microsoft specific C++ stdlib deprecation warnings
		_SCL_SECURE_NO_WARNINGS
	)

	target_link_options(vsm_cmake_options INTERFACE
		# Disable LNK4099: PDB not found
		/IGNORE:4099
	)
endif()

if("${vsm_compiler_frontend}" STREQUAL "GNU")
	target_compile_options(vsm_cmake_options INTERFACE
		-Wall
		-Wextra
		-Wpedantic
		#TODO: Enable warnings as errors
		#-Werror
	)
endif()

include(vsm_add_library)
include(vsm_aggregate_test)
include(vsm_find_sibling)
