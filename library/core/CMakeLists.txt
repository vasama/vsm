cmake_minimum_required(VERSION 3.24)
project(vsm_core)

find_package(vsm.cmake REQUIRED)
vsm_define_package(vsm.core)

vsm_add_library(
	vsm::core

	HEADERS
		include/vsm/allocator_utilities.hpp
		include/vsm/allocator.hpp
		include/vsm/array_view.hpp
		include/vsm/array.hpp
		include/vsm/arrow.hpp
		include/vsm/ascii.hpp
		include/vsm/assert.h
		include/vsm/atomic.hpp
		include/vsm/bit_packing.hpp
		include/vsm/bit.hpp
		include/vsm/concepts.hpp
		include/vsm/coroutine.hpp
		include/vsm/defer.hpp
		include/vsm/exceptions.hpp
		include/vsm/exception_code.hpp
		include/vsm/flags.hpp
		include/vsm/float.hpp
		include/vsm/function_ptr.hpp
		include/vsm/functional.hpp
		include/vsm/hash.hpp
		include/vsm/int128.hpp
		include/vsm/lazy.hpp
		include/vsm/lift.hpp
		include/vsm/literals.hpp
		include/vsm/memory.hpp
		include/vsm/numeric.hpp
		include/vsm/partial.hpp
		include/vsm/platform.h
		include/vsm/preprocessor.h
		include/vsm/relocate.hpp
		include/vsm/sanitizer/address.h
		include/vsm/standard.hpp
		include/vsm/standard/bit.hpp
		include/vsm/standard/memory.hpp
		include/vsm/standard/stdexcept.hpp
		include/vsm/standard/string.hpp
		include/vsm/tag_invoke.hpp
		include/vsm/type_list.hpp
		include/vsm/type_traits.hpp
		include/vsm/utility.hpp

		include/vsm/detail/categories.hpp
		include/vsm/detail/msvc_delete.hpp
		include/vsm/detail/span.hpp
		include/vsm/detail/start_lifetime_as.hpp

		include/vsm/detail/atomic/x86_64.hpp

		include/vsm/detail/bit/generic/compression.hpp
		include/vsm/detail/bit/x86.hpp

		include/vsm/detail/float16.hpp
		include/vsm/detail/float16/generic.h
		include/vsm/detail/float16/x86.h
		include/vsm/detail/float16/x86_32.h
		include/vsm/detail/float16/x86_64.h

		include/vsm/detail/platform/arch.h
		include/vsm/detail/platform/clang.h
		include/vsm/detail/platform/compiler.h
		include/vsm/detail/platform/gcc.h
		include/vsm/detail/platform/gnu.h
		include/vsm/detail/platform/linux.h
		include/vsm/detail/platform/microsoft.h
		include/vsm/detail/platform/msvc.h
		include/vsm/detail/platform/os.h
		include/vsm/detail/platform/win32.h
		include/vsm/detail/platform/x86.h
		include/vsm/detail/platform/x86_32.h
		include/vsm/detail/platform/x86_64.h

		include/vsm/detail/sanitizer/address.h

	SOURCES
		source/vsm/impl/ascii.cpp
		source/vsm/impl/exception_code.cpp
		source/vsm/impl/hash.cpp
		source/vsm/impl/memory.cpp

		source/vsm/impl/sanitizer/address.cpp

	TEST_SOURCES
		source/vsm/test/atomic.cpp
		source/vsm/test/flags.cpp
		source/vsm/test/float.cpp
		source/vsm/test/numeric.cpp
		source/vsm/test/partial.cpp
		source/vsm/test/relocate.cpp
		source/vsm/test/utility.cpp

	TEST_LINK_LIBRARIES
		vsm::testing::core
)

vsm_configure(
	vsm::core
	PLATFORM Windows

	SOURCES
		source/vsm/impl/win32/assert.cpp
)

vsm_configure(
	vsm::core
	PLATFORM Linux

	SOURCES
		source/vsm/impl/linux/assert.cpp
)

vsm_ignore_files(
	source/vsm/impl/ascii/generic.ipp
	source/vsm/impl/ascii/x86_64.ipp
)
