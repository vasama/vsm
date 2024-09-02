message(STATUS "TODO: Configure visualizers")
return()

set(visualizers_path "${CMAKE_}")

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	file(
		GLOB_RECURSE visualizers
		LIST_DIRECTORIES false
		RELATIVE "${visualizers_path}"
		"${visualizers_path}/**/*.natvis"
	)

	foreach(visualizer "${visualizers}")
		message(STATUS "visualizer:'${visualizer}'")
	endforeach()
endif()
