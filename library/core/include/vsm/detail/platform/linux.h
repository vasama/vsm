#define vsm_os_posix 1

#if __has_builtin(__builtin_debugtrap)
#	define vsm_debugbreak() __builtin_debugtrap()
#elif defined(vsm_arch_x86)
#	define vsm_debugbreak() (__extension__({ __asm__("int3"); }))
#endif
