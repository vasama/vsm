find_package(Catch2 REQUIRED)

set(vsm_compiler_frontend ${CMAKE_CXX_COMPILER_ID})
if(DEFINED CMAKE_CXX_COMPILER_FRONTEND_VARIANT)
	set(vsm_compiler_frontend ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT})
endif()

add_library(vsm_cmake_options INTERFACE)

if("${vsm_compiler_frontend}" STREQUAL "MSVC")
	target_compile_options(
		vsm_cmake_options
		INTERFACE
			/W4
			/WX
	
			/wd4100 # Disable C4100: Unreferenced formal parameter
			/wd4200 # Disable C4200: Flexible array member
			/wd4201 # Disable C4201: Unnamed struct/union
			/wd4324 # Disable C4324: Structure was padded due to alignment specifier
			/wd4459 # Disable C4459: Declaration hides global declaration
			/wd4624 # Disable C4624: Destructor was implicitly defined as deleted

			/w14062 # Enable C4062: Unhandled enumerator in switch statement
			/w15038 # Enable C5038: Reordering of non-static data member initialisers
	)

	target_compile_definitions(
		vsm_cmake_options
		INTERFACE
			# Disable Microsoft specific C stdlib deprecation warnings
			_CRT_SECURE_NO_WARNINGS
			# Disable Microsoft specific C++ stdlib deprecation warnings
			_SCL_SECURE_NO_WARNINGS
	)

	target_link_options(
		vsm_cmake_options
		INTERFACE
			/IGNORE:4099 # Disable LNK4099: PDB not found
	)
endif()

if("${vsm_compiler_frontend}" STREQUAL "GNU")
	target_compile_options(
		vsm_cmake_options
		INTERFACE
			-Wall
			-Wextra
			-Wpedantic
			-Werror
	
			# C/C++
			-Wconversion
			-Wduplicated-branches
			-Wformat-overflow=2
			-Wformat-signedness
			-Wformat=2
			-Wimplicit-fallthrough=5
			-Wunreachable-code # GCC accepts but ignores ignores this.
			-Wsign-conversion
	
			# C++
			-Wnon-virtual-dtor
			-Wold-style-cast
			-Wsuggest-override
	)

	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		target_compile_options(
			vsm_cmake_options
			INTERFACE
				-Wmissing-declarations
		)
	endif()

	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		target_compile_options(
			vsm_cmake_options
			INTERFACE
				-Wmissing-prototypes
		)
	endif()
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	target_compile_definitions(
		vsm_cmake_options
		INTERFACE
			"NOMINMAX"
	)
endif()

include(vsm_aggregate_test)
include(vsm_configure)
include(vsm_find_sibling)
