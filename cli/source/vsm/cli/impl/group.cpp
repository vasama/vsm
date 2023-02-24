#include <vsm/impl/group.hpp>

#include <vsm/impl/app.hpp>
#include <vsm/impl/option.hpp>

using namespace vsm;
using namespace vsm::cli;

class group::private_class : public internal_class
{
public:
	app_internal* m_app;
	resource* m_parent;

	resource m_resource;
	resource_locks m_resource_locks;

	size_t m_count = 0;
};

option& group::option(std::string_view const name)
{
	vsm_self(private_class);
	return *self->m_app->create_option(self, self->m_resource, name, false);
}

option& group::flag(std::string_view const name)
{
	vsm_self(private_class);
	return *self->m_app->create_option(self, self->m_resource, name, true);
}

void group::inclusive_lock(resource& resource)
{
	vsm_self(private_class);
	self->m_resource_locks.inclusive(resource);
}

void group::exclusive_lock(resource& resource)
{
	vsm_self(private_class);
	self->m_resource_locks.exclusive(resource);
}

result<void> group_internal::process_argument()
{
	vsm_self(private_class);
	if (self->m_count++ == 0)
	{
		if (base const* const owner = m_resource_locks.lock(*this))
		{
			report_error("'{}' is not compatible with '{}'.\n", m_name, owner->m_name);
		}

		bool const exclusive = any_flags(m_flags, flags::exclusive);
		if (base const* const owner = lock_resource(*self->m_parent, this, exclusive))
		{
			self->m_app->report_error("Option group '{}' inteferes with '{}'.\n", m_name, owner->m_name);
			return vsm::error(error::option_given_too_many_times);
		}
	}
	else
	{
		if (any_flags(m_flags, flags::at_most_one_option))
		{
			self->m_app->report_error("Option group '{}' allows at most one option.\n", m_name);
			return vsm::error(error::option_biven_too_many_times);
		}
	}
	return {};
}

result<void> group_internal::process_completed()
{
	vsm_self(private_class);
	if (self->m_count == 0 && any_flags(m_flags, flags::require_option))
	{
		self->m_app->report_error("Option group '{}' requires at least one option.\n", m_name);
		return vsm::error(error::option_given_too_few_times);
	}
	return {};
}

std::unique_ptr<group_internal> group_internal::create(
	app_internal* const app, resource& parent,
	std::string_view const name)
{
	auto self = std::make_unique<private_class>();

	self->m_app = app;
	self->m_parent = parent;
	self->m_name = name;

	return vsm_move(self);
}
