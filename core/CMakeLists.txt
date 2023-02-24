cmake_minimum_required(VERSION 3.23)

project(vsm_core)

add_library(vsm_core)

target_sources(vsm_core
	PUBLIC
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
		include/vsm/detail/platform/arch.h
		include/vsm/detail/platform/clang.h
		include/vsm/detail/platform/compiler.h
		include/vsm/detail/platform/gcc.h
		include/vsm/detail/platform/gnu.h
		include/vsm/detail/platform/linux.h
		include/vsm/detail/platform/msvc.h
		include/vsm/detail/platform/os.h
		include/vsm/detail/platform/win32.h
		include/vsm/detail/platform/x86.h
		include/vsm/detail/platform/x86_32.h
		include/vsm/detail/platform/x86_64.h

		include/vsm/assert.h
		include/vsm/concepts.hpp
		include/vsm/lift.hpp
		include/vsm/platform.h
		include/vsm/preprocessor.h
		include/vsm/standard.hpp
		include/vsm/tag_invoke.hpp
		include/vsm/type_traits.hpp
		include/vsm/utility.hpp
)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	target_sources(vsm_core
		PRIVATE
			source/vsm/impl/assert.win32.cpp
	)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	target_sources(vsm_core
		PRIVATE
			source/vsm/impl/assert.linux.cpp
	)
endif()

target_include_directories(vsm_core
	PUBLIC
		include
	PRIVATE
		source
)

install(TARGETS vsm_core FILE_SET HEADERS)

if(NOT PROJECT_IS_TOP_LEVEL)
	add_library(vsm::core ALIAS vsm_core)
endif()


#add_executable(vsm_core_test
#	source/vsm/impl/utility.test.cpp
#)
#target_include_directories(vsm_core_test
#	PRIVATE
#		source
#)
#target_link_libraries(vsm_core_test
#	PRIVATE
#		vsm_core
#		Catch2::Catch2WithMain
#)
