#pragma once

#include <vsm/compilers/source.hpp>
#include <vsm/hash_map.hpp>
#include <vsm/hash_set.hpp>
#include <vsm/standard.hpp>
#include <vsm/tag_ptr.hpp>
#include <vsm/vector.hpp>

#include <array>
#include <concepts>
#include <span>
#include <string_view>

namespace vsm::compilers {

class diagnostics;
class diagnostics_module;

enum class diagnostic_level
{
	none,
	message,
	warning,
	error,
};

class diagnostic
{
	std::string_view m_identifier;
	diagnostic_level m_default_level;

public:
	diagnostic_level default_level() const
	{
		return m_default_level;
	}

protected:
	explicit diagnostic(diagnostic_level const level)
		: m_default_level(level)
	{
	}
};

class diagnostic_data
{
	diagnostic const* m_diagnostic;
	source_location m_location;
	diagnostic_level m_level;
	std::byte m_argument_data[];
};
using diagnostic_data_ptr = std::unique_ptr<diagnostic_data>;

using diagnostic_group = std::span<diagnostic const* const>;

class diagnostics_module
{
	vector<diagnostic const*> m_diagnostics;
	vector<key_value_pair<std::string_view, diagnostic_group>> m_names;

public:
	template<typename... Params>
	class diagnostic_type : public diagnostic
	{
	public:
		explicit diagnostic_type(diagnostics_module& module, diagnostic_level const level)
			: diagnostic(level)
		{
			module.m_diagnostics.push_back(this);
		}
	};

	template<size_t Size>
	class diagnostic_name
	{
		diagnostic const* m_diagnostics[Size];

	public:
		explicit diagnostic_name(diagnostics_module& module, std::string_view const name) = delete;

		explicit diagnostic_name(diagnostics_module& module, std::string_view const name, std::derived_from<diagnostic> auto const&... diagnostics)
			: m_diagnostics{ &diagnostics... }
		{
			module.m_names.emplace_back(name, diagnostic_group(m_diagnostics));
		}
	};

	template<typename... Args>
	diagnostic_name(Args const&...) -> diagnostic_name<sizeof...(Args)>;
};

class diagnostics_domain
{
	vector<diagnostics_module const*> m_modules;
	hash_map<std::string_view, diagnostic_group> m_names;
};

class diagnostics_report
{
	vector<diagnostic_data_ptr> m_diagnostics;

public:
};

class diagnostics_scope
{
	using diagnostic_ptr = tag_ptr<diagnostic const, diagnostic_level, diagnostic_level::error>;

	struct key_selector
	{
		vsm_static_operator diagnostic const* operator()(diagnostic const* const ptr) vsm_static_operator_const
		{
			return ptr;
		}

		vsm_static_operator diagnostic const* operator()(diagnostic_ptr const& ptr) vsm_static_operator_const
		{
			return ptr.ptr();
		}
	};

	diagnostics_scope const* m_parent_scope;
	hash_set<diagnostic_ptr, key_selector> m_level;

public:
	void set_level(diagnostic const& diagnostic, diagnostic_level const level)
	{
		m_level.insert_or_assign(diagnostic_ptr(&diagnostic, level));
	}

	diagnostic_level get_level(diagnostic const& diagnostic) const
	{
		for (auto scope = this; scope != nullptr; scope = scope->m_parent_scope)
		{
			if (auto const level = m_level.find(&diagnostic))
			{
				return level->tag();
			}
		}

		return diagnostic.default_level();
	}
};

class diagnostics
{
	diagnostics_report* m_report;
	diagnostics_scope const* m_scope;

public:
	explicit diagnostics(diagnostics_report& report, diagnostics_scope const* const scope)
		: m_report(&report)
		, m_scope(scope)
	{
	}

	template<typename... Params, typename... Args>
	void operator()(source_location const& location, diagnostics_module::diagnostic_type<Params...> const& diagnostic, Args&&... args) const
		requires (sizeof...(Args) == sizeof...(Params)) && (std::is_convertible_v<Args&&, Params> && ...)
	{
		diagnostic_level const level = m_scope != nullptr
			? m_scope->get_level(diagnostic)
			: diagnostic.default_level();
	}
};


#define vsm_compilers_diag_module(module) \
	static ::vsm::compilers::diagnostics_module module = {}

#define vsm_compilers_diag(module, level, diagnostic, ...) \
	static ::vsm::compilers::diagnostics_module::diagnostic_type<__VA_ARGS__> const \
		diagnostic(module, ::vsm::compilers::diagnostic_level::level)

#define vsm_compilers_diag_name(module, name, ...) \
	static ::vsm::compilers::diagnostics_module::diagnostic_name const \
		vsm_pp_cat(vsm_detail_compilers_diag_name_,vsm_pp_counter)(module, name, __VA_ARGS__)

} // namespace vsm::compilers
