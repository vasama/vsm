#pragma once

#include <vsm/cli/base.hpp>

#include <vsm/assert.h>
#include <vsm/concepts.hpp>
#include <vsm/function.hpp>
#include <vsm/partial.hpp>
#include <vsm/result.hpp>
#include <vsm/utility.hpp>

#include <limits>

namespace vsm::cli {

class option : detail::base
{
	std::string_view m_help;
	std::string_view m_help_parameter;

	uint32_t min = 0;
	uint32_t max = std::numeric_limits<uint32_t>::max();

public:
	using handler_type = function<result<void>(std::string_view)>;
	using validator_type = function<result<shared_string>>(shared_string);
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

	template<non_cvref T>
	option& set(T& object, auto&& value)
	{
		vsm_assert(any_flags(m_flags, flags::flag));
		return handle([&object, value = vsm_forward(value)](std::string_view const value) -> result<void>
		{
			vsm_assert(value.empty());
			object = vsm_move(value);
			return {};
		});
	}

	template<non_cvref Container>
	option& push_back(Container& container)
	{
		return handle([&container](std::string_view const value) -> result<void>
		{
			vsm_try(object, cli::parse<typename container::value_type>(value));
			container.push_back(vsm_move(object));
			return {};
		});
	}


	option& handle(handler_type handler);

	option& else_handle(else_handler_type handler);

private:
	void require_internal();

	vsm_partial(option);
};

} // namespace vsm::cli
