# 'user' refers to the package using vsm-conan in its build.

from conan import ConanFile
from conan.errors import ConanException
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout

import json
import os
import re


def _vsm_read_package_info_json(self):
	directory = self.recipe_folder

	while True:
		json_file = os.path.join(directory, "package_info.json")

		if os.path.isfile(json_file):
			with open(json_file, 'r') as file:
				json_data = json.load(file)

			authors = json_data.get("authors")
			license = json_data.get("license")
			src_url = json_data.get("src_url")
			doc_url = json_data.get("doc_url")
			doc_url = doc_url if doc_url else src_url

			def set_attribute(value, attr):
				if value and not hasattr(self, attr):
					setattr(self, value)

			set_attribute(authors, "author")
			set_attribute(license, "license")
			set_attribute(src_url, "url")
			set_attribute(doc_url, "homepage")

			break

		py = os.path.join(directory, "conanfile.py")
		txt = os.path.join(directory, "conanfile.txt")

		if not os.path.isfile(py) and not os.path.isfile(txt):
			break

		directory = os.path.dirname(directory)

class _vsm_requirement:
	def __init__(self, package, version, user, channel, options):
		if channel and not user:
			raise ConanException(f"Requirement specifies CHANNEL without USER")

		self.package = package
		self.version = version
		self.user = user
		self.channel = channel
		self.options = options

	def __str__(self):
		reference = f"{self.package}/{self.version}"
		if self.user:
			reference = f"{reference}@{self.user}"
			if self.channel:
				reference = f"{reference}{self.channel}"
		return reference

def _vsm_cmake_parse_arguments(_0, _1, _N, arguments):
	options = {}
	unparsed = []

	option_map = {}
	argument_index = 0

	def parse_0(option):
		options[option] = True
	def parse_1(option):
		nonlocal argument_index
		options[option] = arguments[argument_index]
		argument_index = argument_index + 1
	def parse_N(option):
		values = []
		nonlocal argument_index
		while argument_index < len(arguments):
			argument = arguments[argument_index]
			if argument in option_map:
				break
			values.append(argument)
			argument_index = argument_index + 1
		options[option] = values

	for x in _0: option_map[x] = parse_0
	for x in _1: option_map[x] = parse_1
	for x in _N: option_map[x] = parse_N

	while argument_index < len(arguments):
		argument = arguments[argument_index]
		argument_index = argument_index + 1
		if argument in option_map:
			option_map[argument](argument)
		else:
			unparsed.append(argument)

	return (options, unparsed)

def _vsm_read_requirements(directory_path):
	try:
		with open(os.path.join(directory_path, "requirements.txt"), "r") as file:
			for requirement in file:
				requirement = requirement.strip()
				requirement = re.sub("#.+$", "", requirement)
				if not requirement: continue

				arguments = requirement.split()
				package = arguments[0].lower()
				version = arguments[1]

				options, arguments = _vsm_cmake_parse_arguments(
					["TEST", "BUILD", "PRIVATE", "HEADER_ONLY"],
					["FIND", "USER", "CHANNEL"],
					[],
					arguments[2:]
				)

				if len(arguments) != 0:
					raise ConanException(f"Requirement contains unrecognized options: {arguments}")

				user = options.pop("USER", None)
				channel = options.pop("CHANNEL", None)

				package_options = {}
				if options.pop("HEADER_ONLY", None):
					package_options["header_only"] = True

				yield (_vsm_requirement(package, version, user, channel, package_options), options)
	except FileNotFoundError:
		pass

def _vsm_define_library_component(component, target_name, target_type, binary_name):
	component.set_property("cmake_target_name", target_name)

	if target_type != "header-library":
		component.libs.append(binary_name)

def _vsm_define_application_component(component, target_name, binary_name):
	pass

class _vsm_component_handler:
	def __init__(self, add_library, add_application):
		self.add_library = add_library
		self.add_application = add_application


class package(ConanFile):
	package_type = "python-require"

	name = "vsm-conan"
	version = "0.1"

# Base class for user package definitions.
class base:
	settings = "os", "compiler", "build_type", "arch"
	generators = "CMakeToolchain", "CMakeDeps"

	exports = (
		"requirements.txt",
	)

	exports_sources = (
		"CMakeLists.txt",
		"requirements.txt",
		"include/*",
		"source/*",
		"visualizers/*",
	)

	def init(self):
		_vsm_read_package_info_json(self)

	def requirements(self):
		for requirement, options in _vsm_read_requirements(self.recipe_folder):
			build = options.get("BUILD") or False
			test = options.get("TEST") or False
			public = not (build or test or options.get("PRIVATE"))

			self.requires(
				str(requirement),
				build=build,
				test=test,
				visible=public,
				transitive_headers=public,
				transitive_libs=public,
				options=requirement.options)

	def build_requirements(self):
		#self.requires("vsm-cmake/0.1", visible=False)
		self.build_requires("vsm-cmake/0.1")

	def layout(self):
		cmake_layout(self)

		if hasattr(self, "_vsm_components"):
			def add_library(name, library_type):
				component_name = name.replace("::", "_")
				self.cpp.source.components[component_name].includedirs = ["include"]

				if library_type is not "header-library":
					self.cpp.build.components[component_name].libdirs = self.cpp.build.libdirs

				if library_type in ("library", "shared-library"):
					self.cpp.build.components[component_name].bindirs = self.cpp.build.bindirs

			def add_application(name, binary_name):
				component_name = name.replace("::", "_")
				self.cpp.build.components[component_name].bindirs = self.cpp.build.bindirs

			self._vsm_components(_vsm_component_handler(add_library, add_application))

		#self.cpp.build.set_property("cmake_build_modules", ["vsm_force_include.cmake"])

	def build(self):
		cmake = CMake(self)
		cmake.configure()
		cmake.build()

	def package(self):
		cmake = CMake(self)
		cmake.install()


	def _vsm_add_library_component(self, name, library_type):
		self._vsm_detail_component_handler.add_library_component(name, library_type)

	def _vsm_add_application_component(self, name, binary_name=None):
		self._vsm_detail_component_handler.add_application_component(name, binary_name)

	def _vsm_add_cmake_build_modules(self):
		# Find and add all scripts in package_folder/cmake_build_modules/
		cmake_build_modules_dir = os.path.join(self.package_folder, "cmake_build_modules")
		if os.path.isdir(cmake_build_modules_dir):
			cmake_build_modules = self.cpp_info.get_property("cmake_build_modules") or []

			for entry in os.scandir(cmake_build_modules_dir):
				cmake_build_modules.append(os.path.join(cmake_build_modules_dir, entry.name))

			if len(cmake_build_modules) > 0:
				self.cpp_info.set_property("cmake_build_modules", cmake_build_modules)

	def package_info(self):
		package_type = str(self.package_type)

		if package_type == "unknown":
			if not hasattr(self, "_vsm_components"):
				raise ConanException("Package with unknown type must define package_info() or _vsm_components()")

			def add_library(name, library_type):
				component_name = name.replace("::", "_")
				_vsm_define_library_component(
					self.cpp_info.components[component_name],
					name,
					library_type,
					component_name)

			def add_application(name, binary_name):
				component_name = name.replace("::", "_")
				_vsm_define_application_component(
					self.cpp_info.components[component_name],
					name,
					binary_name)

			self._vsm_components(_vsm_component_handler(add_library, add_application))
		else:
			target_name = self.name.replace("-", "::")
			if "::" not in target_name:
				target_name = f"{target_name}::{target_name}"
	
			if "library" in package_type:
				_vsm_define_library_component(
					self.cpp_info,
					target_name,
					package_type,
					self.name.replace("-", "_"))

		self._vsm_add_cmake_build_modules()

# Base class for multi-package monorepo roots.
class root:
	settings = "os", "compiler", "build_type", "arch"
	generators = "CMakeToolchain", "CMakeDeps"
	layout = cmake_layout

	def requirements(self):
		self.requires("vsm-cmake/0.1")

		root_prefix_regex = f"{re.escape(self.name)}[\\.\\+\\-].+"
		package_versions = {}

		for entry in os.scandir(self.recipe_folder):
			if not entry.is_dir(): continue

			for requirement, options in _vsm_read_requirements(entry):
				if re.match(root_prefix_regex, requirement.package):
					continue

				previous_version = package_versions.get(requirement.package)
				if previous_version:
					if requirement.version != previous_version:
						raise ConanException(f"Mismatched version requirements: '{requirement.version}' and '{previous_version}'")
					continue
				package_versions[requirement.package] = requirement.version

				self.requires(str(requirement), options=requirement.options)
