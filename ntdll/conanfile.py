from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout

class package(ConanFile):
	settings = "os", "arch"
	exports_sources = "CMakeLists.txt", "parser.c"

	name = "vsm_ntdll"
	version = "0.1"
	
	def generate(self):
		tc = CMakeToolchain(self)
		tc.generate()

	def layout(self):
		cmake_layout(self)

	def build(self):
		cmake = CMake(self)
		cmake.configure()
		cmake.build()

	def package_info(self):
		self.cpp_info.set_property("cmake_file_name", "vsm-ntdll")
		self.cpp_info.set_property("cmake_target_name", "vsm::ntdll")
		self.cpp_info.libs.append("ntdll.lib")
