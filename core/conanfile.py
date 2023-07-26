from conan import ConanFile

class package(ConanFile):
	python_requires = "vsm_tools/0.1"
	python_requires_extend = "vsm_tools.base"

	vsm_name = "vsm::core"
	version = "0.1"

	vsm_libs = ["vsm_core"]

	test_requires = "vsm_test_tools/0.1"
