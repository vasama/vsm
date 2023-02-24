#pragma once

#include <vsm/cli/base.hpp>

#include <vsm/partial.hpp>

namespace vsm::cli {

class group : detail::base
{
	std::string_view m_help;

public:
	group& hide()
	{
		m_flags |= flags::hide;
		return *this;
	}

	group& help(std::string_view const text)
	{
		m_help = text;
		return *this;
	}


	// Only one option may specified within this option group.
	group& single()
	{
		m_flags |= flags::at_most_one_option;
		return *this;
	}

	// No options outside of this group may be specified for the parent command.
	group& exclusive()
	{
		m_flags |= flags::exclusive;
		return *this;
	}

	group& inclusive_lock(resource& resource);
	group& exclusive_lock(resource& resource);


	// Create a new option within this group.
	option& option(std::string_view name);

	// Create a new flag within this group.
	option& flag(std::string_view name);

private:
	vsm_partial(group);
};

} // namespace vsm::cli
