include_guard(GLOBAL)

include(CTest)

set(vsm_compiler_frontend ${CMAKE_CXX_COMPILER_ID})
if(DEFINED CMAKE_CXX_COMPILER_FRONTEND_VARIANT)
	set(vsm_compiler_frontend ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT})
endif()

include("${CMAKE_CURRENT_LIST_DIR}/detail/configure.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/detail/cxx_options.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/detail/package.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/detail/root.cmake")
