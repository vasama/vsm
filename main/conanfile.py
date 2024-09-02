from conan import ConanFile

class package(ConanFile):
	python_requires = "vsm_conan/0.1"
	python_requires_extend = "vsm_conan.base"

	vsm_name = "vsm::main"
	version = "0.1"

	vsm_libs = ["vsm_main"]

	requires = (
		"vsm_result/0.1"
	)
