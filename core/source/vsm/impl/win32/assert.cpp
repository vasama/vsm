#include <vsm/assert.h>

#include <vsm/preprocessor.h>
#include <vsm/weak.h>

#include <cstdio>
#include <cstdlib>

#include <Windows.h>

vsm_clang_diagnostic(ignored "-Wmissing-prototypes")

extern "C"
void __cdecl _assert(char const* message, char const* filename, unsigned line);

extern "C"
bool vsm_weak(vsm_assert_fail)(char const* const file, int const line, char const* const expr)
{
	if (IsDebuggerPresent())
	{
		return true;
	}

	_assert(expr, file, static_cast<unsigned>(line));

	return false;
}

extern "C"
bool vsm_detail_assert_fail(char const* const file, int const line, char const* const expr)
{
	return vsm_assert_fail(file, line, expr);
}
