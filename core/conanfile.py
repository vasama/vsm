from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout

class vsm_core_recipe(ConanFile):
	name = "vsm-core"
	version = "0.1"

	settings = "os", "compiler", "build_type", "arch"
	exports_sources = "CMakeLists.txt", "include/*", "source/*"
	generators = "CMakeToolchain", "CMakeDeps"

	def layout(self):
		cmake_layout(self)

	def build(self):
		cmake = CMake(self)
		cmake.configure()
		cmake.build()

	def package(self):
		cmake = CMake(self)
		cmake.install()

	def package_info(self):
		self.cpp_info.libs = ["vsm_core.lib"]
		self.cpp_info.set_property("cmake_file_name", "vsm-core")
		self.cpp_info.set_property("cmake_target_name", "vsm::core")
