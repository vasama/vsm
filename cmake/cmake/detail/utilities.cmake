macro(vsm_find_program var program)
	find_program("${var}" "${program}" PATHS ENV PATH NO_DEFAULT_PATH ${ARGN})
endmacro()
