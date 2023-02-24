########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(vsm-core_FIND_QUIETLY)
    set(vsm-core_MESSAGE_MODE VERBOSE)
else()
    set(vsm-core_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/vsm-coreTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${vsm-core_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(vsm-core_VERSION_STRING "0.1")
set(vsm-core_INCLUDE_DIRS ${vsm-core_INCLUDE_DIRS_RELEASE} )
set(vsm-core_INCLUDE_DIR ${vsm-core_INCLUDE_DIRS_RELEASE} )
set(vsm-core_LIBRARIES ${vsm-core_LIBRARIES_RELEASE} )
set(vsm-core_DEFINITIONS ${vsm-core_DEFINITIONS_RELEASE} )

# Only the first installed configuration is included to avoid the collision
foreach(_BUILD_MODULE ${vsm-core_BUILD_MODULES_PATHS_RELEASE} )
    message(${vsm-core_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


