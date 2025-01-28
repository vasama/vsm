if(NOT COMMAND vsm_detail_import_visualizers_v1)
	function(vsm_detail_import_visualizers_v1 package)
		list(TRANSFORM ARGN PREPEND "${VSM_PKG_${package}_DIR}/visualizers/")
		if("${CMAKE_MAJOR_VERSION}" EQUAL 3 AND "${CMAKE_MINOR_VERSION}" GREATER_EQUAL 32)
			set_property(DIRECTORY "${PROJECT_SOURCE_DIR}" APPEND PROPERTY VS_SOLUTION_ITEMS ${ARGN})
			source_group("ExternalVisualizers" FILES ${ARGN})
		else()
			if(NOT TARGET vsm-imported-visualizers)
				add_custom_target(vsm-imported-visualizers)
				set_target_properties(vsm-imported-visualizers PROPERTIES FOLDER "ExternalVisualizers")
			endif()
			target_sources(vsm-imported-visualizers PRIVATE ${ARGN})
		endif()
	endfunction()
endif()
