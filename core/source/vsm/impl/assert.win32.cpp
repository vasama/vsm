#include <vsm/assert.h>

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

extern "C"
bool vsm_assert_fail(char const* const file, int const line, char const* const expr)
{
	return vsm_assert_fail_default(file, line, expr);
}

//#pragma comment(linker, "/alternatename:_vsm_assert_fail=_vsm_assert_fail_default")
