#pragma once

#include <source_location>

namespace vsm::unit_test::detail {

class section_set
{
	std::source_location m_source;

public:
	explicit section_set(std::string_view const name, std::source_location const source = std::source_location::current())
		: m_source(source)
	{
		backend::enter_section_set(m_source, name);
	}

	section_set(section_set const&) = delete;
	section_set& operator=(section_set const&) = delete;

	~section_set()
	{
		backend::leave_section_set(m_source);
	}

	explicit operator bool() const
	{
		return true;
	}
};

class section
{
	std::source_location m_source;
	bool m_enter;

public:
	explicit section(std::string_view const name, std::source_location const source = std::source_location::current())
		: m_source(source)
	{
		m_enter = backend::enter_section(m_source, name);
	}

	section(section const&) = delete;
	section& operator=(section const&) = delete;

	~section()
	{
		if (m_enter)
		{
			backend::leave_section(m_source);
		}
	}

	explicit operator bool() const
	{
		return !m_enter;
	}
};

} // namespace vsm::unit_test::detail

#define vsm_detail_unit_test_enter_1(type, variable, ...) \
	if (::vsm::unit_test::detail::type const variable = ::vsm::unit_test::detail::type(__VA_ARGS__)); else

#define vsm_detail_unit_test_enter(type, ...) \
	vsm_detail_unit_test_enter_1(type, vsm_pp_anon(vsm_detail_unit_test_enter_v), __VA_ARGS__)

#define vsm_unit_test_section_set(...) \
	vsm_detail_unit_test_enter(section_set, __VA_ARGS__)

#define vsm_unit_test_section(...) \
	vsm_detail_unit_test_enter(section, __VA_ARGS__)
