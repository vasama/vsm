#pragma once

#include <vsm/preprocessor.h>

namespace vsm::unit_test {

enum class test_result
{
	success                             = 0,
	warning                             = 1,
	failure                             = 2,
};

namespace detail {

struct global_scope
{

};

} // namespace detail
} // namespace vsm::unit_test

#define vsm_detail_unit_test_do_1(counter, ...) \
	static char const vsm_pp_cat(vsm_detail_unit_test_do_,counter) = ((__VA_ARGS__), 0)

#define vsm_detail_unit_test_do(...) \
	vsm_detail_unit_test_do_1(vsm_pp_counter, __VA_ARGS__)

inline vsm::unit_test::detail::global_scope const vsm_detail_unit_test_scope = {};
