from conan import ConanFile

class package(ConanFile):
	python_requires = "vsm-conan/0.1"
	python_requires_extend = "vsm-conan.base"

	name = "vsm-main"
	version = "0.1"
	package_type = "static-library"
