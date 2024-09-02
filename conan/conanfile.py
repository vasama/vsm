# 'user' refers to the package using vsm_conan in its build.

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout

import os

class package(ConanFile):
	package_type = "python-require"

	name = "vsm_conan"
	version = "0.1"

	exports = "cmake/*"

# Base class for user package definitions.
class base:
	settings = "os", "compiler", "build_type", "arch"
	generators = "CMakeDeps"

	exports_sources = (
		"CMakeLists.txt",
		"include/*",
		"source/*",
		"visualizers/*",
	)

	def set_name(self):
		self.name = self.vsm_name.replace("::", "_")

	def build_requirements(self):
		self.requires("vsm_cmake/0.1", visible=False)

		#self.requires(
		#	"vsm_cmake/0.1",
		#	# This requirement is used exclusively during the build.
		#	build=False,
		#	# This requirement is not propagated downstream.
		#	visible=False,
		#	# This requirement does not contain any executables.
		#	run=False,
		#	# This requirement does contain some "headers" (CMake).
		#	headers=True, libs=True)

		self.test_requires("catch2/[^3.4]")

	def generate(self):
		# Path of the vsm-cmake build dependency during the user package build.
		#cmake_path = self.dependencies.get("vsm_cmake", build=True).package_path.as_posix()

		tc = CMakeToolchain(self)
		#tc.variables["CMAKE_PROJECT_TOP_LEVEL_INCLUDES"] = f"{cmake_path}/cmake/vsm.cmake"
		tc.generate()

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
		self.cpp_info.set_property("cmake_file_name", self.vsm_name.replace("::", "-"))

		target_name = self.vsm_name
		if "::" not in target_name:
			target_name = f"{target_name}::{target_name}"

		self.cpp_info.set_property("cmake_target_name", target_name)

		if (hasattr(self, "vsm_libs")):
			self.cpp_info.libs.extend(self.vsm_libs)

		# Find and add all cmake_build_modules/ scripts.
		cmake_build_modules_dir = os.path.join(self.package_folder, "cmake_build_modules")
		if os.path.isdir(cmake_build_modules_dir):
			cmake_build_modules = self.cpp_info.get_property("cmake_build_modules") or []
			for entry in os.scandir(cmake_build_modules_dir):
				cmake_build_modules.append(os.path.join(cmake_build_modules_dir, entry.name))
			self.cpp_info.set_property("cmake_build_modules", cmake_build_modules)
