from conan import ConanFile

class Package(ConanFile):
	python_requires = "vsm-tools/0.1@vasama/testing"
	python_requires_extend = "vsm-tools.PackageBase"

	requires = "vsm-core/0.1@vasama/testing"
