#pragma once

#include <string_view>

namespace vsm::unit_test::detail {

class test_case
{
	backend::test_case m_test_case;
	backend::test_case_instance m_instance;

public:
	explicit test_case(backend::test_case_callback* const callback, std::string_view const name, std::string_view const tags = {})
		: m_test_case
		{
			.name = name,
			.tags = tags,
		}
		, m_instance
		{
			.callback = callback,
		}
	{
		m_test_case.backend_data = backend::register_test_case(m_test_case);
		m_instance.backend_data = backend::register_test_case_instance(m_test_case, m_instance);
	}
};

class test_case_template
{
	backend::test_case m_test_case;

public:
	explicit test_case_template(std::string_view const name, std::string_view const tags = {})
		: m_test_case
		{
			.name = name,
			.tags = tags,
		}
	{
		m_test_case.backend_data = backend::register_test_case(m_test_case);
	}
};

template<typename T>
class test_case_instance
{
	backend::test_case_instance m_instance;

public:
	explicit test_case_instance(test_case_template const& test_case, backend::test_case_callback* const callback, std::string_view const type_argument)
		: m_instance
		{
			.callback = callback,
			.type_argument = type_argument,
		}
	{
		m_instance.backend_data = backend::register_test_case_instances(test_case, m_instance);
	}
};

} // namespace vsm::unit_test::detail

#define vsm_unit_test_case(...) \
	vsm_detail_unit_test_case_1(vsm_pp_counter, __VA_ARGS__)

#define vsm_detail_unit_test_case_1(counter, ...) \
	vsm_detail_unit_test_case_2(\
		vsm_pp_cat(vsm_detail_unit_test_case_f, counter), \
		vsm_pp_cat(vsm_detail_unit_test_case_v, counter), __VA_ARGS__)

#define vsm_detail_unit_test_case_2(function, variable, ...) \
	static void function(); \
	static ::vsm::unit_test::detail::test_case variable(function, __VA_ARGS__); \
	static void function()

#define vsm_unit_test_case_template(name, type, ...) \
	vsm_detail_unit_test_case_template_1(vsm_pp_counter, name, type, __VA_ARGS__)

#define vsm_detail_unit_test_case_template_1(counter, name, type, ...) \
	vsm_detail_unit_test_case_template_2( \
		vsm_pp_cat(vsm_detail_unit_test_case_template_f,name), \
		vsm_pp_cat(vsm_detail_unit_test_case_template_v,name), name, type, __VA_ARGS__)

#define vsm_detail_unit_test_case_template_1(function, variable, name, type, ...) \
	static ::vsm::unit_test::detail::test_case variable(__VA_ARGS__); \
	template<typename type> \
	static void function()

#define vsm_unit_test_case_instance(name, ...) \
	vsm_detail_unit_test_case_instance_1( \
		vsm_pp_cat(vsm_detail_unit_test_case_template_f,name), \
		vsm_pp_cat(vsm_detail_unit_test_case_template_v,name), \
		vsm_pp_anon(vsm_detail_unit_test_case_instance_v), __VA_ARGS__)

#define vsm_detail_unit_test_case_instance_1(function, variable, instance, ...) \
	::vsm::unit_test::detail::test_case_instance instance(variable, function<__VA_ARGS__>, #__VA_ARGS__)
