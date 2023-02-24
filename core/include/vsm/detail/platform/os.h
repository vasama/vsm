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

#ifndef vsm_api_export
#	define vsm_api_export
#endif

#ifndef vsm_api_import
#	define vsm_api_import
#endif
