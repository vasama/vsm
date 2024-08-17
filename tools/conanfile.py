# 'user' refers to the package using vsm_tools in its build.

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
#from conans.errors import ConanException

class package(ConanFile):
	package_type = "python-require"

	name = "vsm_tools"
	version = "0.1"

	exports = "cmake/*"

# Base class for user package definitions.
class base:
	settings = "os", "compiler", "build_type", "arch"
	exports_sources = "CMakeLists.txt", "include/*", "source/*"
	generators = "CMakeDeps"

	def set_name(self):
		self.name = self.vsm_name.replace("::", "_")

#	def _vsm_requires(self, requirement):
#		if not isinstance(requirement, str):
#			raise ConanException()
#
#		requirement.
#
#	def requirements(self):
#		vsm_requires = self.vsm_requires
#
#		if vsm_requires is not None:
#			if isinstance(vsm_requires, str):
#				vsm_requires = tuple(vsm_requires)
#
#			try:
#				for item in vsm_requires:
#					self._vsm_requires(item)
#			except TypeError:
#				raise ConanException("Wrong 'vsm_requires' definition.")

	def build_requirements(self):
		self.requires(
			"vsm_cmake/0.1",
			build=True,
			visible=False,
			run=False)

		self.test_requires("catch2/[^3.4]")

	def generate(self):
		# Path of the vsm-cmake build dependency during the user package build.
		cmake_path = self.dependencies.get("vsm_cmake", build=True).package_path.as_posix()

		tc = CMakeToolchain(self)
		tc.variables["CMAKE_PROJECT_TOP_LEVEL_INCLUDES"] = f"{cmake_path}/cmake/vsm.cmake"
		tc.generate()

	def layout(self):
		cmake_layout(self)

#		gen = self.conf.get("tools.cmake.cmaketoolchain:generator")
#		if gen:
#			multi = "Visual" in gen or "Xcode" in gen or "Multi-Config" in gen
#		else:
#			multi = self.settings.get_safe("compiler") in ("Visual Studio", "msvc")
#
#		self.folders.source = "."
#		if multi:
#			self.folders.build = "build"
#			self.folders.generators = "build/conan"

	def build(self):
		cmake = CMake(self)
		cmake.configure()
		cmake.build()

	def package(self):
		cmake = CMake(self)
		cmake.install()

	def package_info(self):
		self.cpp_info.set_property("cmake_file_name", self.vsm_name.replace("::", "-"))

		target_name = self.vsm_name
		if "::" not in target_name:
			target_name = f"{target_name}::{target_name}"

		self.cpp_info.set_property("cmake_target_name", target_name)

		if (hasattr(self, "vsm_libs")):
			self.cpp_info.libs.extend(self.vsm_libs)
