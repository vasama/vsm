from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout

def requires(self, package):
	if (self.user and self.channel):
		package = f"{package}@{self.user}/{self.channel}"
	self.requires(package)

class vsm_vector_recipe(ConanFile):
	name = "vsm-vector"
	version = "0.1"

	settings = "os", "compiler", "build_type", "arch"
	exports_sources = "CMakeLists.txt", "include/*", "source/*"
	generators = "CMakeToolchain", "CMakeDeps"

	def requirements(self):
		requires("vsm-core/0.1")
		requires("vsm-algorithm/0.1")
		requires("vsm-allocator/0.1")

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
		self.cpp_info.set_property("cmake_file_name", "vsm-vector")
		self.cpp_info.set_property("cmake_target_name", "vsm::vector")
