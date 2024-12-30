set(found_unused_file OFF)

function(load_file_set out_var file_set)
	set(configured_files "")
	set(configured_files_file "${DIRTEST_BINARY_DIR}/vsm_detail_check_directory/${file_set}.txt")

	if(EXISTS "${configured_files_file}")
		file(STRINGS "${configured_files_file}" configured_files)
	endif()

	set("${out_var}" "${configured_files}" PARENT_SCOPE)
endfunction()

load_file_set(ignored_files ignored)

function(find_unused_files directory file_set)
	file(
		GLOB_RECURSE files
		RELATIVE "${DIRTEST_SOURCE_DIR}"
		"${DIRTEST_SOURCE_DIR}/${directory}/**/*")

	load_file_set(configured_files "${file_set}")
	list(REMOVE_ITEM files ${configured_files})
	list(REMOVE_ITEM files ${ignored_files})

	foreach(file ${files})
		message(STATUS "${DIRTEST_SOURCE_DIR} contains unused source file: '${file}'")
		set(found_unused_file ON PARENT_SCOPE)
	endforeach()
endfunction()

find_unused_files("include" "headers")
find_unused_files("source" "sources")
find_unused_files("visualizers" "visualizers")

if(found_unused_file)
	message(SEND_ERROR "${DIRTEST_SOURCE_DIR} contains unused source files")
endif()
