# 'user' refers to the package using vsm.conan in its build.

from conan import ConanFile
from conan.errors import ConanException
from conan.tools.files import load, save, copy
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout

import glob
import json
import os
import re

class Package(ConanFile):
	package_type = "python-require"

	name = "vsm.conan"
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

		parent_directory = os.path.dirname(directory)
		if parent_directory == directory: break
		directory = parent_directory

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

def _vsm_create_package_setup_cmake(conanfile, directory, required=False):
	setup_script = ""
	if os.path.isfile(os.path.join(directory, f"{conanfile.name}-setup.cmake")):
		config = str(conanfile.settings.build_type).upper()
		setup_script = "".join([
			f'set(VSM_PKG_{conanfile.name}_DIR "${{{conanfile.name}_PACKAGE_FOLDER_{config}}}")\n',
			f'include(${{CMAKE_CURRENT_LIST_DIR}}/{conanfile.name}-setup.cmake)\n',
		])

	if setup_script or required:
		save(conanfile, os.path.join(directory, f"{conanfile.name}-setup-conan.cmake"), setup_script)


# Base class for user package definitions.
class base:
	settings = "os", "compiler", "build_type", "arch"

	generators = "CMakeToolchain", "CMakeDeps"

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

	def export(self):
		# Write the full self-contained package_info.json to the export folder:
		json_string = json.dumps(_vsm_read_package_info_json(self.recipe_folder))
		save(self, os.path.join(self.export_folder, "package_info.json"), json_string)

	def export_sources(self):
		# Write the full self-contained package_info.json to the export folder:
		json_string = json.dumps(_vsm_read_package_info_json(self.recipe_folder))
		save(self, os.path.join(self.export_sources_folder, "package_info.json"), json_string)

		copy(self, src=self.recipe_folder, dst=self.export_sources_folder, pattern="CMakeLists.txt")
		copy(self, src=self.recipe_folder, dst=self.export_sources_folder, pattern="cmake/*")
		copy(self, src=self.recipe_folder, dst=self.export_sources_folder, pattern="include/*")
		copy(self, src=self.recipe_folder, dst=self.export_sources_folder, pattern="source/*")
		copy(self, src=self.recipe_folder, dst=self.export_sources_folder, pattern="visualizers/*")

	def layout(self):
		cmake_layout(self)

		# Add source visualizer directory as a resource directory:
		self.cpp.source.resdirs.append("visualizers")

		self.cpp.build.set_property("cmake_build_modules", [f"{self.name}-setup-conan.cmake"])

	def requirements(self):
		for requirement in _vsm_read_package_requirements(self.recipe_folder):
			self.requires(str(requirement), **requirement.arguments)

	def build(self):
		cmake = CMake(self)
		cmake.configure()
		cmake.build()

		_vsm_create_package_setup_cmake(self, self.build_folder, required=True)

	def package(self):
		cmake = CMake(self)
		cmake.install()

		_vsm_create_package_setup_cmake(self, os.path.join(self.package_folder, "cmake"))

	def package_info(self):
		package_type = str(self.package_type)

		if package_type == "" or package_type == "unknown":
			raise ConanException(f"Invalid package_type: '{package_type}'")

		target_name = self.name.replace(".", "::")
		if "::" not in target_name:
			target_name = f"{target_name}::{target_name}"

		if "library" in package_type:
			self.cpp_info.set_property("cmake_target_name", target_name)

			if package_type != "header-library":
				self.cpp_info.libs.append(self.name.replace(".", "_"))

		setup_path = os.path.join("cmake", f"{self.name}-setup-conan.cmake")
		if os.path.isfile(os.path.join(self.package_folder, setup_path)):
			self.cpp_info.set_property("cmake_build_modules", [setup_path])

# Base class for multi-package monorepo roots.
class root:
	settings = "os", "compiler", "build_type", "arch"

	layout = cmake_layout
	generators = "CMakeToolchain", "CMakeDeps"

	def export(self):
		raise ConanException("This conanfile can only be used in editable mode.")

	def requirements(self):
		root_prefix_regex = f"{re.escape(self.name)}[\\.\\+\\-].+"
		package_versions = {}

		def visit_directory(path, filter_root_prefix):
			for requirement in _vsm_read_package_requirements(path, recurse=False):
				if filter_root_prefix and re.match(root_prefix_regex, requirement.package):
					continue

				previous_version = package_versions.get(requirement.package)
				if previous_version is not None:
					if requirement.version != previous_version:
						raise ConanException(f"Mismatched version requirements: '{requirement.version}' and '{previous_version}'")
					continue
				package_versions[requirement.package] = requirement.version

				self.requires(str(requirement), **requirement.arguments)

		visit_directory(self.recipe_folder, filter_root_prefix=False)

		for subdirectory in self.conan_data.get("subdirectories", []):
			for path in glob.iglob(os.path.join(self.recipe_folder, subdirectory)):
				if not os.path.isdir(path):
					continue

				visit_directory(path, filter_root_prefix=True)
