#pragma once

#if __has_include(<expected>)
#	include <expected>
#endif

#ifndef __cpp_lib_expected
#	include <tl/expected.hpp>
#endif

namespace vsm::detail {

#ifdef __cpp_lib_expected
namespace expected_namespace = std;
#else
namespace expected_namespace = tl;
#endif

} // namespace vsm::detail
