from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout

class PackageBase(object):
	settings = "os", "compiler", "build_type", "arch"
	exports_sources = "CMakeLists.txt", "include/*", "source/*"
	generators = "CMakeToolchain", "CMakeDeps"


	def layout(self):
		cmake_layout(self)

	def build(self):
		cmake = CMake(self)
		cmake.configure({
			"CMAKE_PROJECT_TOP_LEVEL_INCLUDES": "D:/Code/vsm/tools/cmake/vsm.cmake"
		})
		cmake.build()

	def package(self):
		cmake = CMake(self)
		cmake.install()

class Package(ConanFile):
	name = "vsm-tools"
	version = "0.1"
