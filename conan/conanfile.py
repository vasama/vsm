# 'user' refers to the package using vsm-conan in its build.

from conan import ConanFile
from conan.errors import ConanException
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout

import os
import re
import yaml

class package(ConanFile):
	package_type = "python-require"

	name = "vsm-conan"
	version = "0.1"


class _vsm_requirement:
	def __init__(self, package, version, user, channel):
		if channel and not user:
			raise ConanException(f"Requirement specifies CHANNEL without USER")

		self.package = package
		self.version = version
		self.user = user
		self.channel = channel

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
					["TEST", "BUILD", "PRIVATE"],
					["FIND", "USER", "CHANNEL"],
					[],
					arguments[2:]
				)

				if len(arguments) != 0:
					raise ConanException(f"Requirement contains unrecognized options: {arguments}")

				user = options.pop("USER", None)
				channel = options.pop("CHANNEL", None)

				yield (_vsm_requirement(package, version, user, channel), options)
	except FileNotFoundError:
		pass

# Base class for user package definitions.
class base:
	settings = "os", "compiler", "build_type", "arch"
	generators = "CMakeToolchain", "CMakeDeps"

	exports = (
		"components.yaml",
		"requirements.txt",
	)

	exports_sources = (
		"CMakeLists.txt",
		"requirements.txt",
		"include/*",
		"source/*",
		"visualizers/*",
	)

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
				transitive_libs=public)

	def build_requirements(self):
		self.requires("vsm-cmake/0.1", visible=False)

	def layout(self):
		cmake_layout(self)

		self.cpp.build.builddirs.append("cmake_build_modules")
		self.cpp.package.builddirs.append("cmake_build_modules")

	def build(self):
		cmake = CMake(self)
		cmake.configure()
		cmake.build()

	def package(self):
		cmake = CMake(self)
		cmake.install()

	def package_info(self):
		package_type = str(self.package_type)

		if package_type == "unknown":
			with open(os.path.join(self.recipe_folder, "components.yaml")) as file:
				components = yaml.safe_load(file)["components"]
				raise ConanException("Not implemented yet")
		else:
			target_name = self.name.replace("-", "::")
			if "::" not in target_name:
				target_name = f"{target_name}::{target_name}"
			self.cpp_info.set_property("cmake_target_name", target_name)

			if "library" in package_type:
				if package_type != "header-library":
					self.cpp_info.libs.append(self.name.replace("-", "_"))
			else:
				self.cpp_info.includedirs = []
				self.cpp_info.libdirs = []

			if package_type not in ("application", "library", "shared-library"):
				self.cpp_info.bindirs = []

		# Find and add all scripts in package_folder/cmake_build_modules/
		cmake_build_modules_dir = os.path.join(self.package_folder, "cmake_build_modules")
		if os.path.isdir(cmake_build_modules_dir):
			cmake_build_modules = self.cpp_info.get_property("cmake_build_modules") or []

			for entry in os.scandir(cmake_build_modules_dir):
				cmake_build_modules.append(os.path.join(cmake_build_modules_dir, entry.name))

			if len(cmake_build_modules) > 0:
				self.cpp_info.set_property("cmake_build_modules", cmake_build_modules)

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

				self.requires(str(requirement))
