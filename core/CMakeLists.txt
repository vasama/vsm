cmake_minimum_required(VERSION 3.23)

project(vsm_core)

vsm_find_sibling(vsm-test_tools)

vsm_add_library(vsm::core
	HEADERS
		include/vsm/assert.h
		include/vsm/atomic.hpp
		include/vsm/concepts.hpp
		include/vsm/defer.hpp
		include/vsm/int128.hpp
		include/vsm/lift.hpp
		include/vsm/platform.h
		include/vsm/preprocessor.h
		include/vsm/standard.hpp
		include/vsm/tag_invoke.hpp
		include/vsm/type_traits.hpp
		include/vsm/utility.hpp

		include/vsm/detail/atomic/x86_64.hpp
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
		include/vsm/detail/categories.hpp

	ADDITIONAL_SOURCES

	TEST_SOURCES
		source/vsm/test/utility.cpp

	TEST_DEPENDENCIES
		vsm::test_tools
)
vsm_configure(vsm::core
	PLATFORM Windows
	SOURCES
		source/vsm/impl/win32/assert.cpp
)
vsm_configure(vsm::core
	PLATFORM Linux
	SOURCES
		source/vsm/impl/linux/assert.cpp
)
