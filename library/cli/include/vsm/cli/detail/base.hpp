#pragma once

#include <vsm/flags.hpp>
#include <vsm/pointer_tag_pair.hpp>

#include <string_view>

#include <cstdint>

namespace vsm::cli {

class app;

namespace detail {

class resource_locks;
class base;

class resource
{
	pointer_tag_pair<detail::base const, bool, 1> m_lock_owner;

public:
	resource() = default;
	resource(resource const&) = delete;
	resource& operator=(resource const&) = delete;

private:
	friend class base;
};

class base
{
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


	[[nodiscard]] static std::string_view get_name(base const& base)
	{
		return base.m_name;
	}

	[[nodiscard]] static base const* lock_resource(
		resource& resource,
		base const& owner,
		bool const exclusive)
	{
		if (resource.m_lock_owner.pointer() == nullptr)
		{
			resource.m_lock_owner = decltype(resource::m_lock_owner)(&owner, exclusive);
		}
		else if (exclusive || resource.m_lock_owner.tag())
		{
			return resource.m_lock_owner.pointer();
		}
		return nullptr;
	}

	friend resource_locks;
	friend app;
};

} // namespace detail

using detail::resource;

} // namespace vsm::cli
