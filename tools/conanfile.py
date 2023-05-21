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
	#generators = "CMakeToolchain", "CMakeDeps"
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

	def generate(self):
		# Path of this file during the user package build.
		tools_path = self.python_requires["vsm_tools"].path.replace('\\', '/')

		tc = CMakeToolchain(self)
		tc.variables["CMAKE_PROJECT_TOP_LEVEL_INCLUDES"] = f"{tools_path}/cmake/vsm.cmake"
		tc.generate()

	def build_requirements(self):
		self.test_requires("catch2/[^3.3]")

	def layout(self):
		cmake_layout(self)

	def build(self):
		# Path of this file during the user package build.
		tools_path = self.python_requires["vsm_tools"].path

		cmake = CMake(self)
		cmake.configure()
		#cmake.configure({
		#	# ./cmake/vsm.cmake is pre-included in all user builds.
		#	"CMAKE_PROJECT_TOP_LEVEL_INCLUDES": f"{tools_path}/cmake/vsm.cmake"
		#})
		cmake.build()

	def package(self):
		cmake = CMake(self)
		cmake.install()

	def package_info(self):
		self.cpp_info.set_property("cmake_file_name", self.vsm_name.replace("::", "-"))
		self.cpp_info.set_property("cmake_target_name", self.vsm_name)
