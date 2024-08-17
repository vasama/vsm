from conan import ConanFile

class package(ConanFile):
	python_requires = "vsm_tools/0.1"
	python_requires_extend = "vsm_tools.base"

	vsm_name = "vsm::test_tools"
	version = "0.1"

	requires = (
		"vsm_allocator/0.1"
	)
