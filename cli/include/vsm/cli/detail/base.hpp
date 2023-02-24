#pragma once

#include <vsm/flags.hpp>

#include <cstdint>

namespace vsm::cli {
namespace detail {

class resource_locks;
class base;

class resource
{
	tag_ptr<detail::base const, bool> m_lock_owner = nullptr;

public:
	resource() = default;
	resource(resource const&) = delete;
	resource& operator=(resource const&) = delete;
};

class base
{
public:
	virtual ~base() = default;

protected:
	enum class flags : uint32_t
	{
		hide                            = 1 << 0,
		exclusive                       = 1 << 1,

		// app
		show_usage                      = 1 << 16,
		require_command                 = 1 << 17,

		// group
		require_option                  = 1 << 16,
		at_most_one_option              = 1 << 17,

		// option
		flag                            = 1 << 16,
		positional                      = 1 << 17,
	};
	vsm_flag_enum_friend(flags);

	flags m_flags = {};
	std::string_view m_name;
	resource m_resource;
};

} // namespace detail

using detail::resource;

} // namespace vsm::cli
