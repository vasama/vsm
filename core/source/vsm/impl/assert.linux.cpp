#include <vsm/assert.h>

#include <cstdlib>

extern "C"
bool vsm_assert_fail(char const* const file, int const line, char const* const expr) __attribute__((weak))
{
	std::abort();
}
