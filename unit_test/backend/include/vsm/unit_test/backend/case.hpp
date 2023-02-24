#pragma once

#include <string_view>

namespace vsm::unit_test::backend {

typedef void test_case_callback();

struct test_case_instance
{
	void* backend_data;
	test_case_callback* callback;
	std::string_view type_argument;
};

struct test_case
{
	void* backend_data;
	std::string_view name;
	std::string_view tags;
};

void* register_test_case(test_case const& test_case);
void* register_test_case_instance(test_case const& test_case, test_case_instance const& instance);

} // namespace vsm::unit_test::backend


