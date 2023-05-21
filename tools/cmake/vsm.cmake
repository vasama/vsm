find_package(Catch2 REQUIRED)

set(vsm_compile_options)

if(MSVC)
	list(APPEND vsm_compile_options "/Zc:preprocessor")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/vsm_add_library.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/vsm_find_sibling.cmake)
