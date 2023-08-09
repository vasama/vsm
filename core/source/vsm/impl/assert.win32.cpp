#include <vsm/assert.h>

#include <vsm/preprocessor.h>

#include <cstdio>
#include <cstdlib>

#include <Windows.h>

extern "C"
bool vsm_assert_fail_default(char const* const file, int const line, char const* const expr)
{
	if (IsDebuggerPresent())
	{
		return true;
	}

	char buffer[1024];
	snprintf(buffer, sizeof(buffer),
		"File: %s\r\n\r\n"
		"Line: %d\r\n\r\n"
		"Expression: (%s)",
		file, line, expr);

	while (true)
	{
		switch (MessageBoxA(NULL, buffer, "Assertion failure.",
			MB_ABORTRETRYIGNORE | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND))
		{
		case IDABORT:
			std::abort();

		case IDRETRY:
			if (IsDebuggerPresent())
			{
				return true;
			}
			continue;

		case IDIGNORE:
			return false;
		}
	}
}

#if vsm_arch_x86_32
#	define vsm_detail_mangle(name) vsm_pp_cat(_, name)
#else
#	define vsm_detail_mangle(name) name
#endif

__pragma(comment(linker, vsm_pp_str(/ALTERNATENAME:vsm_detail_mangle(vsm_assert_fail)=vsm_detail_mangle(vsm_assert_fail_default))))

extern "C"
bool vsm_assert_fail_thunk(char const* const file, int const line, char const* const expr)
{
	return vsm_assert_fail(file, line, expr);
}
