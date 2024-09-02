from conan import ConanFile

class package(ConanFile):
	python_requires = "vsm_conan/0.1"
	python_requires_extend = "vsm_conan.base"

	vsm_name = "vsm::hash_table"
	version = "0.1"

	requires = (
		"vsm_allocator/0.1",
		"vsm_container_core/0.1",
		"vsm_core/0.1",
		"vsm_hash/0.1",
		"vsm_math/0.1",
	)

	test_requires = (
		"vsm_test_tools/0.1",
	)

	vsm_libs = ["vsm_hash_table"]
