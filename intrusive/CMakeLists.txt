cmake_minimum_required(VERSION 3.21)

project(vsm_intrusive)

if(PROJECT_IS_TOP_LEVEL)
	find_package(vsm-core REQUIRED)
	find_package(vsm-container-core REQUIRED)
	find_package(vsm-linear REQUIRED)
	find_package(vsm-tag-ptr REQUIRED)
endif()

add_library(vsm_intrusive)

target_sources(vsm_intrusive
	PUBLIC
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
		include/vsm/intrusive/avl_tree.hpp
		include/vsm/intrusive/heap.hpp
		include/vsm/intrusive/link.hpp
		include/vsm/intrusive/list.hpp
		include/vsm/intrusive/rb_tree.hpp
		include/vsm/intrusive/wb_tree.hpp

	PRIVATE
		source/vsm/intrusive/impl/avl_tree.cpp
		source/vsm/intrusive/impl/heap.cpp
		source/vsm/intrusive/impl/list.cpp
		source/vsm/intrusive/impl/wb_tree.cpp
)

target_include_directories(vsm_intrusive
	PUBLIC
		include
)

target_link_libraries(vsm_intrusive
	PUBLIC
		vsm::core
		vsm::container_core
		vsm::linear
		vsm::tag_ptr
)

install(TARGETS vsm_intrusive FILE_SET HEADERS)


if(NOT PROJECT_IS_TOP_LEVEL)
	add_library(vsm::intrusive ALIAS vsm_intrusive)
endif()


add_executable(vsm_intrusive_test
	source/vsm/intrusive/impl/avl_tree.test.cpp
	source/vsm/intrusive/impl/heap.test.cpp
	source/vsm/intrusive/impl/list.test.cpp
	source/vsm/intrusive/impl/wb_tree.test.cpp
)
target_include_directories(vsm_intrusive_test
	PRIVATE
		source
)
target_link_libraries(vsm_intrusive_test
	PRIVATE
		vsm_intrusive
		Catch2WithMain
)
