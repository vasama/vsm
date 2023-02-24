cmake_minimum_required(VERSION 3.21)

#project(vsm_bits)

add_library(vsm_bits INTERFACE
)
target_include_directories(vsm_bits
	INTERFACE
		include
)
target_compile_features(vsm_bits
	INTERFACE
		cxx_std_20
)

if(PROJECT_IS_TOP_LEVEL)
	add_executable(vsm_bits_test
		source/bit_mask.test.cpp
	)
	target_link_libraries(vsm_bits_test
		PRIVATE
			vsm_bits
			Catch2WithMain
	)
endif()
