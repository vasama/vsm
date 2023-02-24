#include <vsm/impl/option.hpp>

#include <vsm/impl/app.hpp>
#include <vsm/impl/group.hpp>

#include <vsm/vector.hpp>

class option::private_class : public internal_class
{
public:
	app_internal* m_app;
	group_internal* m_group;
	resource* m_parent;

	vector<validator_type> m_validators;
	small_vector<handler_type, 1> m_handlers;
	vector<else_handler_type> m_else_handlers;

	resource_locks m_resource_locks;
};

option& option::handle(handler_type handler)
{
	vsm_self(private_class);
	self->m_handlers.push_back(vsm_move(handler));
	return *this;
}

option& option::else_handle(else_handler_type handler)
{
	vsm_self(private_class);
	self->m_else_handlers.push_back(vsm_move(handler));
	return *this;
}

option& option::hide()
{
	vsm_self(private_class);
	if (no_flags(m_flags, flags::hide))
	{
		size_t& visible_count = any_flags(m_flags, flags::positional)
			? self->m_app->m_visible_positional_count
			: self->m_app->m_visible_option_count;
		vsm_verify(visible_count-- > 0);
	}
	m_flags |= flags::hide;
	return *this;
}

void option::require_internal()
{
	vsm_self(private_class);
	self->m_app->m_has_required_options = true;
}

option& option::inclusive_lock(resource& resource)
{
	vsm_self(private_class);
	self->m_resource_locks.inclusive(resource);
	return *this;
}

option& option::exclusive_lock(resource& resource)
{
	vsm_self(private_class);
	self->m_resource_locks.exclusive(resource);
	return *this;
}

bool option_internal::is_full() const
{
	vsm_self(private_class);
	return self->m_count >= self->m_max;
}

std::unique_ptr<option_internal> option_internal::create(
	app_internal* const app, resource& parent,
	group_internal* const group, bool const flag)
{
	auto self = std::make_unique<private_class>();

	self->m_app = app;
	self->m_group = group;
	self->m_parent = &parent;

	if (flag)
	{
		self->m_flags |= flags::flag;
	}

	return vsm_move(self);
}

result<void> option_internal::process_argument(std::string_view const form, std::string_view const raw_value)
{
	vsm_self(private_class);

	if (self->m_count >= self->m_max)
	{
		self->m_app->report_error("Option '{}' given too many times.", form);
		return vsm::error(error::option_given_too_many_times);
	}

	if (self->m_count++ == 0)
	{
		bool const exclusive = any_flags(m_flags, flags::exclusive);
		if (base const* const owner = lock_resource(*self->m_parent, this, exclusive))
		{
			self->m_app->report_error("Option '{}' interferes with '{}'.\n", form, owner->m_name);
			return vsm:error(error::option_given_too_many_times);
		}

		if (group_internal* const group = self->m_group)
		{
			vsm_try_void(group->process_argument());
		}
	}

	shared_string value = shared_string::borrow(raw_value);
	for (auto const& validator : self->m_validators)
	{
		auto r = validator(vsm_move(value));
		if (!r)
		{
			self->m_app->report_error("Option validation failed: '{}={}'.\n", form, input);
			return vsm::error(r.error());
		}
		value = vsm_move(*r);
	}

	for (auto const& handler : self->m_handlers)
	{
		if (result<void> r = handler(value); !r)
		{
			self->m_app->report_error("Invalid option value: '{}={}'.\n", form, value);
			return r;
		}
	}

	return {};
}

result<void> option_internal::process_completed()
{
	vsm_self(private_class);
	if (any_flags(m_flags, flags::flag) && self->m_count < self->m_min)
	{
		self->m_app->report_error("Option '{}' given too few times.\n", m_name);
		return vsm::error(error::option_given_too_few_times);
	}

	if (self->m_count == 0)
	{
		for (auto const& handler : self->m_else_handlers)
		{
			handler();
		}
	}

	return {};
}
