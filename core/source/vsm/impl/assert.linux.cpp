#include <vsm/assert.h>

#include <cstdlib>

extern "C" __attribute__((weak))
bool vsm_assert_fail(char const* const file, int const line, char const* const expr)
{
	std::abort();
}
