#include <vsm/assert.h>

#include <cstdio>
#include <cstdlib>

extern "C"
bool vsm_assert_fail(char const* const file, int const line, char const* const expr)
{
	// NOLINTNEXTLINE(modernize-use-std-print)
	fprintf(stderr, "Assertion failure: %s\n%s:%d\n", expr, file, line);
	fflush(stderr);

	std::abort();
}
