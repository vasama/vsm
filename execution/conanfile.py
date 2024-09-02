from conan import ConanFile

class package(ConanFile):
	python_requires = "vsm_conan/0.1"
	python_requires_extend = "vsm_conan.base"

	vsm_name = "vsm::execution"
	version = "0.1"

	requires = (
		"vsm_core/0.1",
		"vsm_intrusive/0.1",
		"p2300/1.0@vasama",
	)
