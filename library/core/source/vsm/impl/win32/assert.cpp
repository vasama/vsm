#include <vsm/assert.h>

#include <Windows.h>

extern "C"
void __cdecl _assert(char const* message, char const* filename, unsigned line);

extern "C"
bool vsm_assert_fail(char const* const file, int const line, char const* const expr)
{
	if (IsDebuggerPresent() != 0)
	{
		return true;
	}

	_assert(expr, file, static_cast<unsigned>(line));

	return false;
}
