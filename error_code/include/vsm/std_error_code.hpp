#pragma once

#include <vsm/error_code.hpp>

#include <system_error>

namespace vsm {
namespace detail {

struct std_error_code : std::error_code
{
	using std::error_code::error_code;
};

} // namespace detail

using detail::std_error_code;

} // namespace vsm
