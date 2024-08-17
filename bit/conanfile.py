from conan import ConanFile

class Package(ConanFile):
	python_requires = "vsm_tools/0.1"
	python_requires_extend = "vsm_tools.base"

	vsm_name = "vsm::bit"
	version = "0.1"

	requires = "vsm_core/0.1"

	test_requires = "vsm_test_tools/0.1"
