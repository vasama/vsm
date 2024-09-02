from conan import ConanFile

class package(ConanFile):
	python_requires = "vsm_conan/0.1"
	python_requires_extend = "vsm_conan.base"

	vsm_name = "vsm::intrusive"
	version = "0.1"

	requires = (
		"vsm_core/0.1",
		"vsm_container_core/0.1",
		"vsm_tag_ptr/0.1"
	)

	vsm_libs = ["vsm_intrusive"]
