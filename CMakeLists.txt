cmake_minimum_required(VERSION 3.24)
project(vsm)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Find vsm-cmake using the bootstrap config.
find_package(
	vsm-cmake REQUIRED
	NO_DEFAULT_PATH
	PATHS "${CMAKE_CURRENT_LIST_DIR}/cmake/bootstrap")

vsm_add_root(vsm)

add_subdirectory(algorithm)
add_subdirectory(allocator)
add_subdirectory(any)
add_subdirectory(casting)
add_subdirectory(container_core)
add_subdirectory(core)
add_subdirectory(function)
add_subdirectory(hash)
add_subdirectory(hash_table)
add_subdirectory(intrusive)
add_subdirectory(intrusive_ptr)
add_subdirectory(main)
add_subdirectory(offset_ptr)
add_subdirectory(prop_cv)
add_subdirectory(result)
add_subdirectory(tag_ptr)
add_subdirectory(testing)
add_subdirectory(unique_resource)
add_subdirectory(vector)
