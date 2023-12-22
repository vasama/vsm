#pragma once

#include <vsm/preprocessor.h>

#if defined(__clang__)
#	define vsm_compiler clang
#	define vsm_compiler_clang 1
#	include <vsm/detail/platform/clang.h>
#elif defined(_MSC_VER)
#	define vsm_compiler msvc
#	define vsm_compiler_msvc 1
#	include <vsm/detail/platform/msvc.h>
#elif defined(__GNUC__)
#	define vsm_compiler gcc
#	define vsm_compiler_gcc 1
#	include <vsm/detail/platform/gcc.h>
#else
#	error unsupported compiler
#endif

#include <vsm/detail/platform/compiler.h>


#if defined(__i386__) \
||  defined(__i486__) \
||  defined(__i586__) \
||  defined(__i686__) \
||  defined(_M_IX86)
#	define vsm_arch x86_32
#	define vsm_arch_x86 1
#	define vsm_arch_x86_32 1
#	include <vsm/detail/platform/x86_32.h>
#else
#	define vsm_arch_x86_32 0
#endif

#if defined(__x86_64) \
||  defined(__x86_64__) \
||  defined(__amd64) \
||  defined(__amd64__) \
||  defined(_M_X64)
#	define vsm_arch x86_64
#	define vsm_arch_x86 1
#	define vsm_arch_x86_64 1
#	include <vsm/detail/platform/x86_64.h>
#else
#	define vsm_arch_x86_64 0
#endif

#ifndef vsm_arch
#	error unsupported cpu architecture
#endif

#include <vsm/detail/platform/arch.h>


#if defined(_WIN32)
#	define vsm_os win32
#	define vsm_os_win32 1
#	include <vsm/detail/platform/win32.h>
#elif defined(__linux__)
#	define vsm_os linux
#	define vsm_os_linux 1
#	include <vsm/detail/platform/linux.h>
#else
#	error unsupported operating system
#endif

#include <vsm/detail/platform/os.h>
