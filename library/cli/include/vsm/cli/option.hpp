#pragma once

#include <vsm/cli/detail/base.hpp>
#include <vsm/cli/parse.hpp>

#include <vsm/assert.h>
#include <vsm/concepts.hpp>
#include <vsm/function.hpp>
#include <vsm/partial.hpp>
#include <vsm/result.hpp>
#include <vsm/utility.hpp>

#include <limits>

namespace vsm::cli {

class app;

class option : detail::base
{
	std::string_view m_help;
	std::string_view m_help_parameter;

	uint32_t m_min = 0;
	uint32_t m_max = std::numeric_limits<uint32_t>::max();

	uint32_t m_count = 0;

public:
	using validator_type = function<result<std::string>(std::string_view) const>;
	using handler_type = function<result<void>(std::string_view)>;
	using else_handler_type = function<void()>;


	option& hide();

	option& help(std::string_view const text)
	{
		m_help = text;
		return *this;
	}

	option& parameter(std::string_view const text)
	{
		m_help_parameter = text;
		return *this;
	}


	option& require()
	{
		return min(1);
	}

	option& single()
	{
		return max(1);
	}

	option& min(uint32_t const min)
	{
		vsm_assert(min < m_max);
		if (m_min == 0 && min > 0)
		{
			require_internal();
		}
		m_min = std::max(m_min, min);
		return *this;
	}

	option& max(uint32_t const max)
	{
		vsm_assert(0 < max && m_min <= max);
		m_max = std::min(m_max, max);
		return *this;
	}


	option& exclusive()
	{
		m_flags |= flags::exclusive;
		return *this;
	}

	option& inclusive_lock(resource& resource);
	option& exclusive_lock(resource& resource);


	template<non_cvref T>
	option& set(T& object)
	{
		max(1);
		return handle([&object](std::string_view const value) -> result<void>
		{
			vsm_try_assign(object, cli::parse<T>(value));
			return {};
		});
	}

	template<non_cvref Container>
	option& push_back(Container& container)
	{
		return handle([&container](std::string_view const value) -> result<void>
		{
			vsm_try(object, cli::parse<typename Container::value_type>(value));
			container.push_back(vsm_move(object));
			return {};
		});
	}


	option& handle(handler_type handler);

	option& handle_else(else_handler_type handler);

private:
	void require_internal();

	vsm_partial(option);

	friend app;
};

} // namespace vsm::cli
