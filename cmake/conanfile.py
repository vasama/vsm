from conan import ConanFile
from conan.tools.files import copy
from os.path import join

class package(ConanFile):
	no_copy_source = True

	name = "vsm_cmake"
	version = "0.1"

	def package(self):
		copy(
			self,
			"*.cmake",
			join(self.source_folder, "cmake"),
			join(self.package_folder, "cmake"))

	def package_info(self):
		self.cpp_info.includedirs = []
		self.cpp_info.libdirs = []
		self.cpp_info.builddirs.append("cmake")
