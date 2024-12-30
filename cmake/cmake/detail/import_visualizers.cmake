function(vsm_detail_import_visualizers target)
	if(NOT TARGET vsm-visualizers)
		add_custom_target(vsm-visualizers)
	endif()

	list(TRANSFORM ARGN PREPEND "${VSM_PACKAGE_DIR}/")
	target_sources(vsm-visualizers ${ARGN})
endfunction()
