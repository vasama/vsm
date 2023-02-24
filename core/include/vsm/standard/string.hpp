#pragma once

#include <vsm/assert.h>
#include <vsm/utility.hpp>

#include <string>

namespace vsm {

template<typename Char, typename Traits, typename Operation>
static void resize_and_overwrite(
	std::basic_string<Char, Traits>& string,
	size_t const new_size,
	Operation&& operation)
{
#ifdef __cpp_lib_string_resize_and_overwrite
	string.resize_and_overwrite(new_size, vsm_forward(operation));
#else
	string.resize(new_size);
	size_t const final_size = vsm_forward(operation)(
		string.data(),
		string.size());
	vsm_assert(final_size < new_size);
	string.resize(final_size);
#endif
}

} // namespace vsm
