# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(vsm-core_FRAMEWORKS_FOUND_RELEASE "") # Will be filled later
conan_find_apple_frameworks(vsm-core_FRAMEWORKS_FOUND_RELEASE "${vsm-core_FRAMEWORKS_RELEASE}" "${vsm-core_FRAMEWORK_DIRS_RELEASE}")

set(vsm-core_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET vsm-core_DEPS_TARGET)
    add_library(vsm-core_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET vsm-core_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Release>:${vsm-core_FRAMEWORKS_FOUND_RELEASE}>
             $<$<CONFIG:Release>:${vsm-core_SYSTEM_LIBS_RELEASE}>
             $<$<CONFIG:Release>:>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### vsm-core_DEPS_TARGET to all of them
conan_package_library_targets("${vsm-core_LIBS_RELEASE}"    # libraries
                              "${vsm-core_LIB_DIRS_RELEASE}" # package_libdir
                              "${vsm-core_BIN_DIRS_RELEASE}" # package_bindir
                              "${vsm-core_LIBRARY_TYPE_RELEASE}"
                              "${vsm-core_IS_HOST_WINDOWS_RELEASE}"
                              vsm-core_DEPS_TARGET
                              vsm-core_LIBRARIES_TARGETS  # out_libraries_targets
                              "_RELEASE"
                              "vsm-core"    # package_name
                              "${vsm-core_NO_SONAME_MODE_RELEASE}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${vsm-core_BUILD_DIRS_RELEASE} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Release ########################################
    set_property(TARGET vsm::core
                 PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Release>:${vsm-core_OBJECTS_RELEASE}>
                 $<$<CONFIG:Release>:${vsm-core_LIBRARIES_TARGETS}>
                 APPEND)

    if("${vsm-core_LIBS_RELEASE}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET vsm::core
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     vsm-core_DEPS_TARGET
                     APPEND)
    endif()

    set_property(TARGET vsm::core
                 PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Release>:${vsm-core_LINKER_FLAGS_RELEASE}> APPEND)
    set_property(TARGET vsm::core
                 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Release>:${vsm-core_INCLUDE_DIRS_RELEASE}> APPEND)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET vsm::core
                 PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Release>:${vsm-core_LIB_DIRS_RELEASE}> APPEND)
    set_property(TARGET vsm::core
                 PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Release>:${vsm-core_COMPILE_DEFINITIONS_RELEASE}> APPEND)
    set_property(TARGET vsm::core
                 PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Release>:${vsm-core_COMPILE_OPTIONS_RELEASE}> APPEND)

########## For the modules (FindXXX)
set(vsm-core_LIBRARIES_RELEASE vsm::core)
