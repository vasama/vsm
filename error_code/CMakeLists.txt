cmake_minimum_required(VERSION 3.21)

#project(vsm_error_code)

add_library(vsm_error_code
	source/error_code.cpp
	source/error_code.std.cpp
)
target_include_directories(vsm_error_code
	PUBLIC
		include
)
target_compile_features(vsm_error_code
	PUBLIC
		cxx_std_20
)

if(PROJECT_IS_TOP_LEVEL)
	add_executable(vsm_error_code_test
		source/error_code.test.cpp
	)
	target_link_libraries(vsm_error_code_test
		PRIVATE
			vsm_error_code
			Catch2WithMain
	)
endif()
