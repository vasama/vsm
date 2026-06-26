#!/usr/bin/pwsh

conan create (Resolve-Path "$PSScriptRoot/../conan")
conan create (Resolve-Path "$PSScriptRoot/../cmake")
