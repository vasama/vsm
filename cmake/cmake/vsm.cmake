include_guard(GLOBAL)

include(CTest)

find_package(Catch2 REQUIRED QUIET)

# Store the full path of this directory.
set(vsm_detail_cmake_root ${CMAKE_CURRENT_LIST_DIR})

set(vsm_compiler_frontend ${CMAKE_CXX_COMPILER_ID})
if(DEFINED CMAKE_CXX_COMPILER_FRONTEND_VARIANT)
	set(vsm_compiler_frontend ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT})
endif()

add_library(vsm_detail_cxx_options INTERFACE)

if("${vsm_compiler_frontend}" STREQUAL "MSVC")
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set(gnu_prefix "/clang:")
	else()
		target_compile_options(
			vsm_detail_cxx_options
			INTERFACE
				/W4
				/WX
	
				/wd4100 # Disable C4100: Unreferenced formal parameter
				/wd4141 # Disable C4141: inline used more than once (triggered by __forceinline)
				/wd4200 # Disable C4200: Flexible array member
				/wd4201 # Disable C4201: Unnamed struct/union
				/wd4324 # Disable C4324: Structure was padded due to alignment specifier
				/wd4459 # Disable C4459: Declaration hides global declaration
				/wd4624 # Disable C4624: Destructor was implicitly defined as deleted
	
				/w14062 # Enable C4062: Unhandled enumerator in switch statement
				#/w14365 # Enable C4365: Conversion from T1 to T2, signed/unsigned mismatch
				/w14555 # Enable C4555: Result of expression not used
				/w15038 # Enable C5038: Reordering of non-static data member initialisers
		)
	endif()
endif()

if("${vsm_compiler_frontend}" STREQUAL "GNU" OR DEFINED "gnu_prefix")
	target_compile_options(
		vsm_detail_cxx_options
		INTERFACE
			${gnu_prefix}-Wall
			${gnu_prefix}-Wextra
			${gnu_prefix}-Wpedantic
			${gnu_prefix}-Werror

			# Disable default warnings
			${gnu_prefix}-Wno-c99-extensions
			${gnu_prefix}-Wno-gnu-anonymous-struct
			${gnu_prefix}-Wno-invalid-offsetof
			${gnu_prefix}-Wno-missing-field-initializers #TODO: Consider turning this back on
			${gnu_prefix}-Wno-nested-anon-types
			${gnu_prefix}-Wno-unused-parameter
			${gnu_prefix}-Wno-zero-length-array

			# C/C++
			${gnu_prefix}-Wconversion
			${gnu_prefix}-Wformat=2
			${gnu_prefix}-Wunreachable-code # GCC accepts but ignores ignores this.
			${gnu_prefix}-Wsign-conversion

			# C++
			${gnu_prefix}-Wnon-virtual-dtor
			${gnu_prefix}-Wold-style-cast
			${gnu_prefix}-Wsuggest-override
	)

	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		target_compile_options(
			vsm_detail_cxx_options
			INTERFACE
				${gnu_prefix}-Wduplicated-branches
				${gnu_prefix}-Wformat-overflow=2
				${gnu_prefix}-Wformat-signedness
				${gnu_prefix}-Wimplicit-fallthrough=5
				${gnu_prefix}-Wmissing-declarations
		)
	endif()

	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		target_compile_options(
			vsm_detail_cxx_options
			INTERFACE
				${gnu_prefix}-Wformat-overflow
				${gnu_prefix}-Wmissing-prototypes
		)
	endif()
endif()

if("${CMAKE_CXX_COMPILER_LINKER_ID}" STREQUAL "MSVC")
	target_link_options(
		vsm_detail_cxx_options
		INTERFACE
			/IGNORE:4099 # Disable LNK4099: PDB not found
	)
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	target_compile_definitions(
		vsm_detail_cxx_options
		INTERFACE
			NOMINMAX # Disable min/max macros in Windows.h.
			_CRT_SECURE_NO_WARNINGS # Disable Microsoft specific C stdlib deprecation warnings
			_SCL_SECURE_NO_WARNINGS # Disable Microsoft specific C++ stdlib deprecation warnings
	)
endif()

include(vsm_cmake)
include(vsm_configure)
include(vsm_find_requirements)
include(vsm_root)
