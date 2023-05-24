from conan import ConanFile

class package(ConanFile):
	python_requires = "vsm_tools/0.1"
	python_requires_extend = "vsm_tools.base"

	vsm_name = "vsm::result"
	version = "0.1"

	requires = "vsm_core/0.1", "tl-expected/1.1.1"
	test_requires = "vsm_test_tools/0.1"
