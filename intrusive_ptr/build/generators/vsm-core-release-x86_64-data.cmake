########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(vsm-core_COMPONENT_NAMES "")
set(vsm-core_FIND_DEPENDENCY_NAMES "")

########### VARIABLES #######################################################################
#############################################################################################
set(vsm-core_PACKAGE_FOLDER_RELEASE "D:/Code/vsm/core")
set(vsm-core_BUILD_MODULES_PATHS_RELEASE )


set(vsm-core_INCLUDE_DIRS_RELEASE "${vsm-core_PACKAGE_FOLDER_RELEASE}/./include")
set(vsm-core_RES_DIRS_RELEASE )
set(vsm-core_DEFINITIONS_RELEASE )
set(vsm-core_SHARED_LINK_FLAGS_RELEASE )
set(vsm-core_EXE_LINK_FLAGS_RELEASE )
set(vsm-core_OBJECTS_RELEASE )
set(vsm-core_COMPILE_DEFINITIONS_RELEASE )
set(vsm-core_COMPILE_OPTIONS_C_RELEASE )
set(vsm-core_COMPILE_OPTIONS_CXX_RELEASE )
set(vsm-core_LIB_DIRS_RELEASE "${vsm-core_PACKAGE_FOLDER_RELEASE}/build/Release")
set(vsm-core_BIN_DIRS_RELEASE )
set(vsm-core_LIBRARY_TYPE_RELEASE UNKNOWN)
set(vsm-core_IS_HOST_WINDOWS_RELEASE 1)
set(vsm-core_LIBS_RELEASE vsm_core.lib)
set(vsm-core_SYSTEM_LIBS_RELEASE )
set(vsm-core_FRAMEWORK_DIRS_RELEASE )
set(vsm-core_FRAMEWORKS_RELEASE )
set(vsm-core_BUILD_DIRS_RELEASE )
set(vsm-core_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(vsm-core_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${vsm-core_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${vsm-core_COMPILE_OPTIONS_C_RELEASE}>")
set(vsm-core_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${vsm-core_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${vsm-core_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${vsm-core_EXE_LINK_FLAGS_RELEASE}>")


set(vsm-core_COMPONENTS_RELEASE )