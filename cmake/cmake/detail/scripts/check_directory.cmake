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

set(unused_files)
function(find_unused_files directory file_set)
	file(
		GLOB_RECURSE files
		RELATIVE "${DIRTEST_SOURCE_DIR}"
		"${DIRTEST_SOURCE_DIR}/${directory}/**/*")

	load_file_set(configured_files "${file_set}")
	list(REMOVE_ITEM files ${configured_files})
	list(REMOVE_ITEM files ${ignored_files})

	foreach(file ${files})
		set(absolute_path "${DIRTEST_SOURCE_DIR}/${file}")

		cmake_path(
			RELATIVE_PATH absolute_path
			BASE_DIRECTORY "${DIRTEST_ROOT_DIR}"
			OUTPUT_VARIABLE relative_path
		)

		list(APPEND unused_files "${relative_path}")
		set(unused_files "${unused_files}" PARENT_SCOPE)
	endforeach()
endfunction()

find_unused_files("include" "headers")
find_unused_files("source" "sources")
find_unused_files("visualizers" "visualizers")

if(NOT "${unused_files}" STREQUAL "")
	message("unused_files='${unused_files}'")
	list(TRANSFORM unused_files PREPEND "  ")
	list(TRANSFORM unused_files APPEND "\n")
	string(JOIN "" unused_files ${unused_files})

	message(AUTHOR_WARNING "The project contains unused source files:\n${unused_files}")
endif()
