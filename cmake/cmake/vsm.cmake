find_package(Catch2 REQUIRED)

add_library(vsm_cmake_options INTERFACE)

if(MSVC)
	target_compile_options(vsm_cmake_options INTERFACE
		/permissive-
		/Zc:preprocessor
		/Zc:__cplusplus
		/W4
		# Disable C4324: Structure was padded due to alignment specifier
		/wd4324
	)

	target_compile_definitions(vsm_cmake_options INTERFACE
		# Disable Microsoft specific C stdlib deprecation warnings
		_CRT_SECURE_NO_WARNINGS
		# Disable Microsoft specific C++ stdlib deprecation warnings
		_SCL_SECURE_NO_WARNINGS
	)

	target_link_options(vsm_cmake_options INTERFACE
		# Disable LNK4099: PDB not found
		/IGNORE:4099
	)
endif()

include(vsm_add_library)
include(vsm_aggregate_test)
include(vsm_find_sibling)
