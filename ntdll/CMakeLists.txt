cmake_minimum_required(VERSION 3.23)

project(vsm_ntdll)

set(ntdll_dir "System32")
if(${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL AMD64 AND ${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86)
	set(ntdll_dir "SysWOW64")
endif()

set(ntdll_dll "C:\\Windows\\${ntdll_dir}\\ntdll.dll")
if(NOT EXISTS ${ntdll_dll})
	message(FATAL_ERROR "NTDLL.DLL not found at ${ntdll_dll}")
endif()

add_executable(vsm_ntdll_parser parser.c)

set(ntdll_txt "${CMAKE_CURRENT_BINARY_DIR}/ntdll.txt")
set(ntdll_def "${CMAKE_CURRENT_BINARY_DIR}/ntdll.def")
set(ntdll_lib "${CMAKE_CURRENT_BINARY_DIR}/ntdll.lib")
set(ntdll_exp "${CMAKE_CURRENT_BINARY_DIR}/ntdll.exp")

set(lib_machine x64)
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86)
	set(lib_machine x86)
endif()

add_custom_command(
	OUTPUT "${ntdll_txt}"
	MAIN_DEPENDENCY "${ntdll_dll}"
	COMMAND dumpbin "/NOLOGO" "/EXPORTS" "${ntdll_dll}" > "${ntdll_txt}"
)
add_custom_command(
	OUTPUT "${ntdll_def}"
	MAIN_DEPENDENCY "${ntdll_txt}"
	COMMAND vsm_ntdll_parser < "${ntdll_txt}" > "${ntdll_def}"
)
add_custom_command(
	OUTPUT "${ntdll_lib}"
	OUTPUT "${ntdll_exp}"
	MAIN_DEPENDENCY "${ntdll_def}"
	COMMAND lib "/DEF:${ntdll_def}" "/OUT:${ntdll_lib}" "/MACHINE:x64"
)
add_custom_target(vsm_ntdll_implib DEPENDS "${ntdll_lib}")

add_library(vsm_ntdll SHARED IMPORTED)
set_target_properties(vsm_ntdll PROPERTIES IMPORTED_IMPLIB "${ntdll_lib}")
add_dependencies(vsm_ntdll vsm_ntdll_implib)
add_library(vsm::ntdll ALIAS vsm_ntdll)
