from conan import ConanFile

class package(ConanFile):
	python_requires = "vsm_conan/0.1"
	python_requires_extend = "vsm_conan.base"

	vsm_name = "vsm::shared_vector"
	version = "0.1"

	requires = (
		"vsm_core/0.1",
		"vsm_allocator/0.1",
		"vsm_vector/0.1",
	)

	test_requires = "vsm_test_tools/0.1"
