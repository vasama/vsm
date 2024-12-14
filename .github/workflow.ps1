#!/usr/bin/pwsh

param(
	[string]$Tool,
	[string]$Config,

	[string]$ConanBuild=$null,
	[string]$Step=$null
)

function Invoke-NativeCommand($Command) {
	& $Command $Args

	if (!$?) {
		exit $LastExitCode
	}
}

if ($IsWindows) { $OS = 'windows' }
if ($IsLinux) { $OS = 'linux' }

$Analysis = ".github/analysis/$Tool.ps1"
if (!(Test-Path -PathType Leaf $Analysis)) {
	$Analysis = $null
}

$ConanProfile = "vsm-$OS-$Tool"
$ConanSettings = @("build_type=$Config")

if ($IsWindows) {
	$ConanSettings += @("compiler.runtime_type=$Config")
}

$ConanArguments = @($ConanSettings | ForEach-Object { "-s=$_" })

$ConanPreset = "$ConanProfile-$($Config.ToLowerInvariant())"
$CMakePreset = "conan-$ConanPreset"

if (!$Step -or $Step -eq 'conan-config') {
	Invoke-NativeCommand conan config install .github/conan
}

if (!$Step -or $Step -eq 'conan-install') {
	if ($ConanBuild) {
		$ConanArguments += @("-b=$ConanBuild")
	}

	Invoke-NativeCommand conan install '-pr:a' $ConanProfile @ConanArguments .
}

if (!$Step -or $Step -eq 'cmake-configure') {
	Invoke-NativeCommand cmake --preset $CMakePreset
}

if ((!$Step -and $Analysis -eq $null) -or $Step -eq 'cmake-build') {
	Invoke-NativeCommand cmake --build --preset $CMakePreset
}

if ((!$Step -and $Analysis -eq $null) -or $Step -eq 'ctest') {
	Invoke-NativeCommand ctest --preset $CMakePreset --output-on-failure
}

if ((!$Step -and $Analysis -ne $null) -or $Step -eq 'analyze') {
	$CompileCommands = "build/$ConanPreset/compile_commands.json"
	$CompileCommands = ConvertFrom-Json $(Get-Content -Raw $CompileCommands)

	$Results = $CompileCommands | ForEach-Object -Parallel {
		$Output = & $using:Analysis "build/$using:ConanPreset" $_

		return @{
			File = $_.file;
			ExitCode = $LastExitCode;
			Output = $Output;
		}
	}

	$Status = $True
	foreach ($Result in $Results) {
		if ($Result.ExitCode -ne 0) {
			$Result.Output
			$Status = $False
		}
	}

	if (!$Status) {
		exit 1
	}
}
