from conan import ConanFile

class package(ConanFile):
	python_requires = "vsm_conan/0.1"
	python_requires_extend = "vsm_conan.base"

	vsm_name = "vsm::numeric"
	version = "0.1"

	requires = (
		"vsm_core/0.1",
		"vsm_result/0.1",
	)
