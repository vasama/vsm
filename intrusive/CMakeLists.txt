cmake_minimum_required(VERSION 3.23)

project(vsm_intrusive)

vsm_find_sibling(vsm-core)
vsm_find_sibling(vsm-container-core)
vsm_find_sibling(vsm-linear)
vsm_find_sibling(vsm-tag-ptr)

vsm_add_library(vsm_intrusive
	EXPORT_ALIAS vsm::intrusive

	HEADERS
		include/vsm/intrusive/avl_tree.hpp
		include/vsm/intrusive/heap.hpp
		include/vsm/intrusive/link.hpp
		include/vsm/intrusive/list.hpp
		include/vsm/intrusive/rb_tree.hpp
		include/vsm/intrusive/wb_tree.hpp

	SOURCES
		source/vsm/intrusive/impl/avl_tree.cpp
		source/vsm/intrusive/impl/heap.cpp
		source/vsm/intrusive/impl/list.cpp
		source/vsm/intrusive/impl/wb_tree.cpp

	HEADER_DEPENDENCIES
		vsm::core
		vsm::container_core
		vsm::linear
		vsm::tag_ptr

	TEST_SOURCES
		source/vsm/intrusive/test/avl_tree.cpp
		source/vsm/intrusive/test/heap.cpp
		source/vsm/intrusive/test/list.cpp
		source/vsm/intrusive/test/wb_tree.cpp
)
