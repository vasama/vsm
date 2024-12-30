#ifndef vsm_os_win32
#	define vsm_os_win32 0
#endif

#ifndef vsm_os_posix
#	define vsm_os_posix 0
#endif

#ifndef vsm_os_linux
#	define vsm_os_linux 0
#endif

#ifndef vsm_debugbreak
#	define vsm_debugbreak() ((void)0)
#endif

#ifndef vsm_dll_export
#	define vsm_dll_export
#endif

#ifndef vsm_dll_import
#	define vsm_dll_import
#endif
