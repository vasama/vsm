#include <vsm/assert.h>

#include <vsm/preprocessor.h>

#include <cstdio>
#include <cstdlib>

#include <Windows.h>

extern "C"
void __cdecl _assert(char const* message, char const* filename, unsigned line);

extern "C"
bool vsm_assert_fail_default(char const* const file, int const line, char const* const expr)
{
	if (IsDebuggerPresent())
	{
		return true;
	}

	_assert(expr, file, static_cast<unsigned>(line));

	return false;
}


#if vsm_arch_x86_32
#	define vsm_detail_mangle(name) vsm_pp_cat(_, name)
#else
#	define vsm_detail_mangle(name) name
#endif

__pragma(comment(linker, vsm_pp_str(/ALTERNATENAME:vsm_detail_mangle(vsm_assert_fail)=vsm_detail_mangle(vsm_assert_fail_default))))
