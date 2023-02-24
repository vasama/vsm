add_library(vsm_unit_test_frontend)

target_sources(vsm_unit_test_frontend
	PUBLIC
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
		include/vsm/unit_test/detail/abort.hpp
		include/vsm/unit_test/detail/assert.hpp
		include/vsm/unit_test/detail/branch.hpp
		include/vsm/unit_test/detail/case.hpp
		include/vsm/unit_test/detail/core.hpp
		include/vsm/unit_test/detail/generate.hpp
		include/vsm/unit_test/detail/section.hpp

	PRIVATE
)

target_include_directories(vsm_unit_test_frontend
	PUBLIC
		include
)

target_link_libraries(vsm_unit_test_frontend
	PUBLIC
		vsm::core
)

if(NOT PROJECT_IS_TOP_LEVEL)
	add_library(vsm::unit_test::frontend ALIAS vsm_unit_test_frontend)
endif()
