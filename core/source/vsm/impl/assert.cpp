#include <vsm/assert.h>

extern "C"
bool vsm_detail_assert_fail(char const* const file, int const line, char const* const expr)
{
	return vsm_assert_fail(file, line, expr);
}
