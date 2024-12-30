# 'user' refers to the package using vsm-conan in its build.

from conan import ConanFile
from conan.errors import ConanException
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout

import glob
import json
import os
import re

class Package(ConanFile):
	package_type = "python-require"

	name = "vsm-conan"
	version = "0.1"

	license = "MIT"
	author = "Lauri Vasama"
	url = "https://github.com/vasama/vsm"
	homepage = "https://github.com/vasama/vsm"


def _vsm_read_package_info_json(directory, recurse=True):
	json_layers = []

	while directory:
		json_file = os.path.join(directory, "package_info.json")

		if os.path.isfile(json_file):
			with open(json_file, 'r') as file:
				json_data = json.load(file)

			json_layers.append(json_data)

			if not recurse or json_data.get("is_root"):
				break

		directory = os.path.dirname(directory)

	if len(json_layers) == 0:
		raise ConanException("package_info.json not found")

	json_layers.reverse()

	full_json_data = {}
	for json_data in json_layers:
		for key, value in json_data.items():
			if isinstance(value, list):
				full_json_data.setdefault(key, []).extend(value)
			else:
				full_json_data[key] = value

	return full_json_data

class _vsm_requirement:
	def __init__(self, json):
		self.package = json["package"]
		self.version = json["version"]

		self.user = None
		self.channel = None

		self.arguments = {}
		def set_argument(type, name, argument_name=None):
			if argument_name is None: argument_name = name

			value = json.get(name)
			if value is not None:
				if not isinstance(value, type):
					raise RuntimeError(f"Mistyped requirement option: {name}")

				self.arguments[name] = value

		set_argument(bool, "host", "build")
		set_argument(bool, "test")
		set_argument(bool, "visible")

	def __str__(self):
		reference = f"{self.package}/{self.version}"
		if self.user:
			reference = f"{reference}@{self.user}"
			if self.channel:
				reference = f"{reference}{self.channel}"
		return reference

def _vsm_read_package_requirements(directory, recurse=True):
	json = _vsm_read_package_info_json(directory, recurse)

	for json_require in json.get("requirements", []):
		yield _vsm_requirement(json_require)


# Base class for user package definitions.
class base:
	settings = "os", "compiler", "build_type", "arch"

	layout = cmake_layout
	generators = "CMakeToolchain", "CMakeDeps"

	exports = (
		"requirements.txt",
		"package_info.json",
	)

	exports_sources = (
		"CMakeLists.txt",
		"requirements.txt",
		"package_info.json",
		"include/*",
		"source/*",
		"visualizers/*",
	)

	def init(self):
		json = _vsm_read_package_info_json(self.recipe_folder, recurse=False)

		def set_attr(name, attr=None):
			if attr is None:
				attr = name

			value = json.get(name)
			if value is not None:
				setattr(self, attr, value)

		set_attr("package", "name")
		set_attr("version")
		set_attr("authors", "author")
		set_attr("license")
		set_attr("website", "homepage")
		set_attr("package_type")

	def requirements(self):
		for requirement in _vsm_read_package_requirements(self.recipe_folder):
			self.requires(str(requirement), **requirement.arguments)

	def build(self):
		cmake = CMake(self)
		cmake.configure()
		cmake.build()

	def package(self):
		cmake = CMake(self)
		cmake.install()

	def package_info(self):
		package_type = str(self.package_type)

		if package_type == "" or package_type == "unknown":
			raise ConanException(f"Invalid package_type: '{package_type}'")

		target_name = self.name.replace("-", "::")
		if "::" not in target_name:
			target_name = f"{target_name}::{target_name}"

		if "library" in package_type:
			self.cpp_info.set_property("cmake_target_name", target_name)

			if package_type != "header-library":
				self.cpp_info.libs.append(self.name.replace("-", "_"))

		# Find and add all scripts in package_folder/cmake_build_modules/
		cmake_build_modules_dir = os.path.join(self.package_folder, "cmake_build_modules")
		if os.path.isdir(cmake_build_modules_dir):
			cmake_build_modules = self.cpp_info.get_property("cmake_build_modules") or []

			for entry in os.scandir(cmake_build_modules_dir):
				cmake_build_modules.append(os.path.join(cmake_build_modules_dir, entry.name))

			if len(cmake_build_modules) > 0:
				self.cpp_info.set_property("cmake_build_modules", cmake_build_modules)

# Base class for multi-package monorepo roots.
class root:
	settings = "os", "compiler", "build_type", "arch"

	layout = cmake_layout
	generators = "CMakeToolchain", "CMakeDeps"

	def requirements(self):
		self.requires("vsm-cmake/0.1")

		root_prefix_regex = f"{re.escape(self.name)}[\\.\\+\\-].+"
		package_versions = {}

		for subdirectory in self.conan_data.get("subdirectories", []):
			for path in glob.iglob(subdirectory):
				if not os.path.isdir(path):
					continue

				for requirement in _vsm_read_package_requirements(path, recurse=False):
					if re.match(root_prefix_regex, requirement.package):
						continue

					previous_version = package_versions.get(requirement.package)
					if previous_version:
						if requirement.version != previous_version:
							raise ConanException(f"Mismatched version requirements: '{requirement.version}' and '{previous_version}'")
						continue
					package_versions[requirement.package] = requirement.version

					self.requires(str(requirement), **requirement.arguments)
