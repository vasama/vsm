cmake_minimum_required(VERSION 3.24)
project(vsm)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Find vsm-cmake using the bootstrap config.
find_package(
	vsm-cmake REQUIRED
	NO_DEFAULT_PATH
	PATHS "${CMAKE_CURRENT_LIST_DIR}/cmake/bootstrap")

vsm_add_root(vsm)

add_subdirectory(testing)
