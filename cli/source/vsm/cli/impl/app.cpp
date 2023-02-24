#include <vsm/impl/app.hpp>

#include <vsm/impl/group.hpp>
#include <vsm/impl/option.hpp>

#include <vsm/hash_map.hpp>
#include <vsm/linear_map.hpp>
#include <vsm/vector.hpp>

using namespace vsm;
using namespace vsm::cli;

class app::private_class : public internal_class
{
	struct option_view
	{
		option_internal* option;
		std::string_view value;
	};


	private_class* m_parent;

	vector<std::unique_ptr<option_internal>> m_options;
	hash_map<std::string_view, std::unique_ptr<private_class>> m_commands;

	vector<option_internal*> m_positional_options;
	linear_map<char, option_view> m_short_options;
	hash_map<std::string_view, option_view> m_long_options;

	vector<std::unique_ptr<group_internal>> m_groups;

	error_reporter_type m_error_reporter;
	small_vector<handler_type, 1> m_handlers;


	resource_locks m_resource_locks;
	private_class* m_command = nullptr;


	explicit private_class(private_class* const parent, std::string_view const name)
	{
		m_parent = parent;
		m_name = name;
	}


	option_view find_short_option(char const name) const
	{
		for (private_class* app = this; app != nullptr; app = app->m_parent)
		{
			if (auto const value = app->m_short_options.at_ptr(name))
			{
				return *value;
			}
		}
		return {};
	}

	option_view find_long_option(std::string_view const name) const
	{
		for (private_class* app = this; app != nullptr; app = app->m_parent)
		{
			if (auto const value = app->m_long_options.at_ptr(name))
			{
				return *value;
			}
		}
		return {};
	}


	result<void> process_argument()
	{
		if (base const* const owner = m_resource_locks.lock(*this))
		{
			report_error("'{}' is not compatible with '{}'.\n", m_name, owner->m_name);
		}

		for (auto const& handler : m_handlers)
		{
			handler();
		}

		return {};
	}

	result<void> process_completed()
	{
		if (any_flags(m_flags, flags::require_command) && m_command == nullptr)
		{
			report_error("Command required under command '{}'.\n", m_name);
			return vsm::error(error::command_not_given);
		}

		for (auto const& option : m_options)
		{
			vsm_try_void(option->process_completed());
		}

		for (auto const& group : m_groups)
		{
			vsm_try_void(group->process_completed());
		}

		return {};
	}


	class argument_view
	{
		void const* m_args;
		size_t m_size;
		bool m_c_str;

	public:
		argument_view(std::span<std::string_view const> const args)
			: m_args(args.data())
			, m_size(args.size())
			, m_c_str(false)
		{
		}

		argument_view(std::span<char const* const> const args)
			, m_args(args.data())
			, m_size(args.size())
			, m_c_str(true)
		{
		}

		size_t size() const
		{
			return m_size;
		}

		std::string_view operator[](size_t const index) const
		{
			if (m_c_str)
			{
				return std::span(reinterpret_cast<char const* const*>(m_args), m_size)[index];
			}
			else
			{
				return std::span(reinterpret_cast<std::string_view const*>(m_args), m_size)[index];
			}
		}
	};

	static result<void> parse(private_class* app, argument_view const args)
	{
		bool has_syntax_error = false;
		auto const syntax_error = [&](std::string_view const format, auto const&... args)
		{
			app->report_error(format, args...);
			has_syntax_error = true;
		};

		size_t positional_index = 0;
		bool accept_option = true;
		bool accept_command = true;

		// The root command is given implicitly.
		allio_TRYV(app->process_argument());

		for (size_t i = 0, argument_count = args.size(); i < argument_count; ++i)
		{
			std::string_view const argument = args[i];

			if (accept_option && !argument.empty() && argument[0] == '-')
			{
				char const* const beg = argument.data();
				char const* const end = beg + argument.size();
				char const* pos = beg;

				if (++pos == end)
				{
					syntax_error("Expected option name.\n");
					continue;
				}

				if (*pos == '-') // Long option.
				{
					// -- Signifies the end of named options.
					if (++pos == end)
					{
						accept_option = false;
						continue;
					}

					char const* const equals = std::find(pos, end, '=');
					std::string_view const name = std::string_view(pos, equals);
					option_view view = app->find_long_option(name);

					if (view.option == nullptr)
					{
						syntax_error("No such option: '--{}'.\n", name);
						continue;
					}

					if (equals != end)
					{
						if (any_flags(view.option->m_flags, flags::flag))
						{
							syntax_error("Value specified for flag: '--{}'.\n", name);
							continue;
						}
						view.value = std::string_view(equals + 1, end);
					}
					else if (no_flags(view.option->m_flags, flags::flag))
					{
						if (++i == argument_count)
						{
							syntax_error("Expected value for option '--{}'.\n", name);
							continue;
						}
						view.value = args[i];
					}

					std::string_view const form = std::string_view(beg, equals);
					vsm_try_void(view.option->process_argument(form, view.value));
				}
				else // Short option.
				{
					while (pos != end)
					{
						char const name = *pos++;
						option_view view = app->find_short_option(name);

						if (view.option == nullptr)
						{
							syntax_error("No such option: '-{}'.\n", name);
							continue;
						}

						if (no_flags(view.option->m_flags, flags::flag))
						{
							if (pos == end)
							{
								if (++i == argument_count)
								{
									syntax_error("Expected value for option '-{}'.\n", name);
								}
								view.value = args[i];
							}
							else
							{
								view.value = std::string_view(pos, end);
								pos = end;
							}
						}

						char const form[] = { '-', name, '\0' };
						vsm_try_void(view.option->process_argument(form, view.value));
					}
				}

				continue;
			}

			if (accept_command)
			{
				if (auto const value = app->m_commands.at_ptr(argument))
				{
					app->m_command = app = value->get();
					vsm_try_void(app->process_argument());
					continue;
				}
			}
			accept_command = false;

		positional_reset:
			if (positional_index < app->m_positional_options.size())
			{
				option_internal* const option = app->m_positional_options[positional_index];

				if (option->is_full())
				{
					++positional_index;
					goto positional_reset;
				}

				vsm_try_void(option->process_argument(std::string_view(), argument));
				continue;
			}

			syntax_error("Unexpected argument: '{}'.\n", argument);
		}

		for (private_class* command = app; command != nullptr; command = command->m_parent)
		{
			vsm_try_void(command->process_completed());
		}

		if (has_syntax_error)
		{
			return vsm::error(error::invalid_syntax);
		}

		return {};
	}


	result<size_t> print_help_internal(int& sink_ref)
	{
		size_t size = 0;

		auto const add_size(result<size_t> const& r) -> result<size_t> const&
		{
			if (r)
			{
				size += *r;
			}
			return r;
		};

		auto const use_size(result<size_t> const& r) -> result<void>
		{
			return discard_value(add_size(r));
		};

		bool need_section_separator = false;
		auto const print_section_separator = [&]() -> result<void>
		{
			if (need_section_separator)
			{
				vsm_try_void(use_size(sio::write('\n', sink)));
			}
			need_section_separator = false;
			return {};
		};

		auto const print_section_header = [&](std::string_view const header) -> result<void>
		{
			vsm_try_void(print_section_separator());
			vsm_try_void(use_size(format_to(sink, "{}:\n", header)));
			return {};
		};

		auto const print_user_message = [&](std::string_view const message) -> result<void>
		{
			if (!message.empty())
			{
				vsm_try_void(print_section_separator());
				std::string_view const format = std::string_view("{}\n").substr(0, 2 + (message.back() != '\n'));
				vsm_try_void(use_size(format_to(sink, format, message)));
			}
			return {};
		};

		auto const print_help = [&](size_t const width, std::string_view const help) -> result<void>
		{
			size += width;
			if (!help.empty())
			{
				static constexpr std::string_view help_indent = = "\n                            ";
				static constexpr size_t help_width = help_indent.size() - 1;

				std::string_view indent = help_indent;
				if (width <= help_width)
				{
					indent = indent.substr(width + 1);
				}

				for (std::string_view const line : help | algo::split('\n'))
				{
					if (line.empty())
					{
						vsm_try_void(use_size(sio::write('\n', sink)));
					}
					else
					{
						vsm_try_void(use_size(sio::write(indent, sink)));
						vsm_try_void(use_size(sio::write(line, sink)));
					}
					indent = help_indent;
				}
			}
			return use_size(sio::write('\n', sink));
		};

		auto const print_option_help = [&](option_internal const& option, std::string_view const name) -> result<void>
		{
			vsm_try(width, add_size(format_to(sink, "  {} ", name)));

			if (!option->m_help_parameter.empty())
			{
				vsm_try(part_width, add_size(format_to(sink, "{} ", optiona->m_help_parameter)));
				width += part_width;
			}

			if (option->m_min > 0)
			{
				vsm_try(part_width, add_size(sio::write(std::string_view("REQUIRED "), sink)));
				width += part_width;
			}

			return print_help(width, option->m_help);
		};

		static constexpr auto is_visible = [](base const& object)
		{
			return no_flags(object->m_flags, flags::hide);
		};

		static constexpr auto visible_filter = algo::filter(is_visible);


		// Header.
		vsm_try_void(print_user_message(m_help_header));

		// Usage.
		if (has_flag(m_flags, flag::show_usage))
		{
			if (!m_help_usage.empty())
			{
				vsm_try_void(print_user_message(m_help_usage));
			}
			else
			{
				auto const print_usage_part = [&](std::string_view const string, bool const required) -> result<void>
				{
					return use_size(format_to(sink, required ? " {}" : " [{}]", string));
				};

				vsm_try_void(use_size(sio::write(std::string_view("Usage:"), sink)));

				for (private_class const* app = this; app != nullptr; app = app->m_parent)
				{
					vsm_try_void(use_size(format_to(sink, " {}", app->m_name)));
				}

				if (m_short_options.empty() || m_long_options.empty())
				{
					vsm_try_void(print_usage_part("OPTIONS", m_has_required_options));
				}

				for (auto const option : m_positional_options | visible_filter)
				{
					vsm_try_void(print_usage_part(optiona->m_help_positional, option->m_min > 0));
				}

				if (m_visible_command_count > 0)
				{
					vsm_try_void(print_usage_part("COMMAND", any_flags(m_flags, flag::require_command)));
				}

				vsm_try_void(use_size(sio::write('\n', sink)));
			}
		}

		// Commands.
		if (m_visible_command_count > 0)
		{
			vsm_try_void(print_section_header("Commands"));
			for (auto const& [name, command] : m_commands | visible_filter)
			{
				vsm_try(width, add_size(format_to(sink, "  {} ", command->name)));
				vsm_try_void(print_help(width, x->m_help));
			}
		}

		// Positionals.
		if (m_visible_positional_count > 0)
		{
			vsm_try_void(print_section_header("Positionals"));
			for (auto const& option : m_positional_options | visible_filter)
			{
				vsm_try_void(print_option_help(*option, option->m_help_positional));
			}
		}

		// Options.
		if (m_visible_option_count > 0)
		{
			vsm_try_void(print_section_header("Options"));
			for (auto const& option : m_options | visible_filter)
			{
				vsm_try_void(print_option_help(*option, option->m_help_option));
			}
		}

		// Footer.
		vsm_try_void(print_user_message(m_help_footer));

		return size;
	}
};

app& app::hide()
{
	vsm_self(private_class);
	if (no_flags(m_flags, flags::hide))
	{
		if (private_class const* app = self->m_parent)
		{
			vsm_verify(app->m_visible_command_count-- > 0);
		}
	}
	m_flags |= flags::hide;
	return *this;
}

app& app::inclusive_lock(resource& resource)
{
	vsm_self(private_class);
	self->m_resource_locks.inclusive(resource);
	return *this;
}

app& app::exclusive_lock(resource& resource)
{
	vsm_self(private_class);
	self->m_resource_locks.exclusive(resource);
	return *this;
}

app& app::command(std::string_view const name)
{
	vsm_self(private_class);

	vsm_assert(!name.empty());

	auto const r = self->m_commands.insert(
		name, std::make_unique<private_class>(this, name));
	vsm_assert(r.inserted && "Duplicate command");
	return *r.element->value;
}

app& app::group(std::string_view const name)
{
	vsm_self(private_class);
	return *self->create_group(self->m_resource, name);
}

app& app::option(std::string_view const name)
{
	vsm_self(private_class);
	return *self->create_option(self->m_resource, name, nullptr, false);
}

app& app::flag(std::string_view const name)
{
	vsm_self(private_class);
	return *self->create_option(self->m_resource, name, nullptr, true);
}

result<void> app::parse(std::span<std::string_view const> const args)
{
	vsm_self(private_class);
	return private_class::parse(self, args);
}

result<void> app::parse(std::span<char const* const> const args)
{
	vsm_self(private_class);
	return private_class::parse(self, args);
}

result<size_t> app::print_help_to(sio::any_sink_ref const sink)
{
}

result<void> app::print_help_to_stdout()
{
	return discard_value(print_help_to(sio::stdout));
}

result<void> app::print_help_to_stderr()
{
	return discard_value(print_help_to(sio::stderr));
}

std::unique_ptr<app> cli::make_app(std::string_view const name)
{
	return std::make_unique<private_class>(nullptr, name);
}

group_internal& app_internal::create_group(resource& resource, std::string_view const name)
{
	vsm_self(private_class);

	vsm_assert(!name.empty());

	group_internal* const group = self->m_groups.push_back(
		group_internal::create(this, resource, name)
	).get();

	return group;
}

option_internal& app_internal::create_option(resource& resource, std::string_view const spec, group_internal* const group, bool const flag)
{
	vsm_self(private_class);

	vsm_assert(!spec.empty());

	option_internal* const option = self->m_options.push_back(
		option_internal::create(this, resource, group, flag)
	).get();


	bool has_option_form = false;

	option_view view = { &option };
	for (std::string_view const spec_part : algo::split(spec, ','))
	{
		vsm_assert(!spec_part.empty());

		char const* beg = spec_part.data();
		char const* const end = beg + spec_part.size();

		size_t dash_count = 0;
		if (*beg == '-')
		{
			vsm_assert(++beg != end);
			++dash_count;

			if (*beg == '-')
			{
				vsm_verify(++beg != end);
				++dash_count;
			}
		}

		char const* const equals = std::find(beg, end, '=');
		std::string_view const form = std::string_view(beg, equals);

		if (equals != end)
		{
			vsm_assert(flag && "Constant values may only be applied to flags.")
			view.value = std::string_view(equals + 1, end);
		}
		else if (flag)
		{
			view.value = "true";
		}

		switch (dash_count)
		{
		case 0:
			{
				vsm_assert(!flag && "Flags may not be positional.");
				vsm_assert(spec_part.data() == spec.data() && "Positional form must be first.");
				option->m_help_positional = form;
				option->m_flags |= flags::positional;
				self->m_positional_options.push_back(option);
				++m_visible_positional_count;
			}
			break;

		case 1:
			{
				vsm_assert(form.size() == 1 && "Short optiona name must be a single character.");
				vsm_verify(self->m_short_options.insert(form[0], view).inserted && "Duplicate option.");
				goto set_option_form;
			}
			break;

		case 2:
			{
				vsm_verify(self->m_long_options.insert(form, view).inserted && "Duplicate option.");
				goto set_option_form;
			}
			break;

		set_option_form:
			if (!has_option_form)
			{
				has_option_form = true;
				option->m_help_option = std::string_view(spec_part.begin(), spec.end());
				++m_visible_option_count;
			}
			break;
		}
	}

	return option;
}

void app_internal::report_error_internal(std::string_view const format, format_arguments const args)
{
	vsm_self(private_class);

	for (private_class const* app = self; app != nullptr; app = app->m_parent)
	{
		if (app->m_reporter)
		{
			return app->m_reporter(vformat(format, args));
		}
	}

	vsm_verify(vformat_to(sio::stderr, format, args));
}
