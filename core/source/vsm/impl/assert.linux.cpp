#include <vsm/assert.h>

#include <cstdio>
#include <cstdlib>

extern "C" __attribute__((weak))
bool vsm_assert_fail(char const* const file, int const line, char const* const expr)
{
	fprintf(stderr, "Assertion failure: %s\n%s:%d\n", expr, file, line);
	fflush(stderr);

	std::abort();
}

extern "C"
bool vsm_detail_assert_fail(char const* const file, int const line, char const* const expr)
{
	return vsm_assert_fail(file, line, expr);
}
