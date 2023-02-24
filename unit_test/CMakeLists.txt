cmake_minimum_required(VERSION 3.23.0)

project(vsm_unit_test)

if(PROJECT_IS_TOP_LEVEL)
	find_package(vsm-core REQUIRED)
endif()

add_library(vsm::unit_test INTERFACE)

target_link_libraries(vsm::unit_test
	INTERFACE
		vsm_unit_test_frontend
		vsm_unit_test_backend
)
