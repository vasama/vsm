#pragma once

#include <vsm/assert.h>

#include <concepts>

namespace vsm {

template<std::integral Output, std::signed_integral Input>
Output truncate(Input const input)
	requires (sizeof(Output) < sizeof(Input))
{
	using output_unsigned = std::make_unsigned_t<Output>;
	using input_unsigned = std::make_unsigned_t<Input>;
	vsm_assert(static_cast<input_unsigned>(input) <= std::numeric_limits<output_unsigned>::max());
	return static_cast<Output>(input);
}

} // namespace vsm
