#pragma once

#include <vsm/platform.h>

#if vsm_os_win32
	#include <vsm/preprocessor.h>

	#if vsm_arch_x86_32
		#define vsm_detail_weak_mangle(f) vsm_pp_cat(_, f)
	#else
		#define vsm_detail_weak_mangle(f) f
	#endif

	#define vsm_detail_weak_pragma(f, d) \
		__pragma(comment(linker, vsm_pp_str(/ALTERNATENAME:vsm_detail_weak_mangle(f)=vsm_detail_weak_mangle(d))))

	#define vsm_detail_weak(f) \
		vsm_pp_cat(vsm_detail_weak_f, f)

	#define vsm_weak(f) \
		vsm_detail_weak_pragma(f, vsm_detail_weak(f)) vsm_detail_weak(f)
#else
	#define vsm_weak(f) \
		__attribute__((weak)) f
#endif
