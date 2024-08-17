from conan import ConanFile

class package(ConanFile):
	python_requires = "vsm_tools/0.1"
	python_requires_extend = "vsm_tools.base"

	vsm_name = "vsm::log"
	version = "0.1"

	requires = "vsm_core/0.1"
