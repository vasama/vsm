include_guard(GLOBAL)

function(vsm_find_sibling sibling)
	if(PROJECT_IS_TOP_LEVEL)
		find_package(${sibling} REQUIRED ${ARGN})
	endif()
endfunction()
