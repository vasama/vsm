from conan import ConanFile
from conan.tools.files import copy
from os.path import join

class package(ConanFile):
	no_copy_source = True

	name = "vsm-cmake"
	version = "0.1"

	requires = (
		"catch2/[^3.4]",
	)

	def layout(self):
		self.folders.generators = "build"

	def package(self):
		copy(
			self,
			"*.cmake",
			join(self.source_folder, "cmake", "cmake"),
			join(self.package_folder, "cmake"))

	def package_info(self):
		self.cpp_info.includedirs = []
		self.cpp_info.libdirs = []
		self.cpp_info.builddirs.append("cmake")
		self.cpp_info.set_property("cmake_file_name", "vsm-cmake")
		#TODO: Should not define a CMake target at all.
		self.cpp_info.set_property("cmake_target_name", "vsm::cmake")
		self.cpp_info.set_property("cmake_build_modules", ["cmake/vsm.cmake"])
