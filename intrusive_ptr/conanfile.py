from conan import ConanFile

class Package(ConanFile):
	python_requires = "vsm_tools/0.1"
	python_requires_extend = "vsm_tools.base"

	requires = "vsm_core/[^0.1]"
