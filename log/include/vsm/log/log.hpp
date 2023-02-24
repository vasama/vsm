#pragma once

#include <vsm/log/level.hpp>
#include <vsm/utility.hpp>

namespace vsm {

using log_string_type = char const*;

struct log_entry
{
	log_string_type file;
	int line;
};

namespace detail {

template<static_string File, int line>
inline constexpr log_entry log_entry_instance
{
};

template<static_string Name, typename T>
struct log_argument
{
	T const& value;
};

template<static_string Name>
struct log_argument_name
{
	template<typename T>
	vsm_static_operator vsm_always_inline log_argument<Name, T> operator()(T const& value) vsm_static_operator_const
	{
		return log_argument<Name, T>{ value };
	}
};

namespace log_literal {

template<static_string Name>
vsm_always_inline log_argument_name<Name> operator""_()
{
	return {};
}

} //namespace log_literal
} // namespace detail

#define vsm_log(logger, level, message, ...) ( \
		[&] vsm_always_inline (auto&& vsm_log_logger) -> void \
		{ \
			static constexpr ::vsm::log_level vsm_log_level = (level); \
			if (vsm_log_level <= vsm_as_const(vsm_log_logger).get_log_level()) \
			{ \
				using namespace ::vsm::detail::log_literal; \
				vsm_log_logger.log( \
					::vsm::detail::log_entry_instance<__FILE__, __LINE__, message> \
					__VA_ARGS__); \
			} \
		}((logger)) \
	)

#if vsm_config_log_level >= vsm_log_level_fatal
#	define vsm_log_fatal(...) vsm_log(::vsm::log_level::fatal, __VA_ARGS__)
#else
#	define vsm_log_fatal(...) ((void)0)
#endif

#if vsm_config_log_level >= vsm_log_level_error
#	define vsm_log_error(...) vsm_log(::vsm::log_level::error, __VA_ARGS__)
#else
#	define vsm_log_error(...) ((void)0)
#endif

#if vsm_config_log_level >= vsm_log_level_warn
#	define vsm_log_warn(...) vsm_log(::vsm::log_level::warn, __VA_ARGS__)
#else
#	define vsm_log_warn(...) ((void)0)
#endif

#if vsm_config_log_level >= vsm_log_level_info
#	define vsm_log_info(...) vsm_log(::vsm::log_level::info, __VA_ARGS__)
#else
#	define vsm_log_info(...) ((void)0)
#endif

#if vsm_config_log_level >= vsm_log_level_debug
#	define vsm_log_debug(...) vsm_log(::vsm::log_level::debug, __VA_ARGS__)
#else
#	define vsm_log_debug(...) ((void)0)
#endif

#if vsm_config_log_level >= vsm_log_level_trace
#	define vsm_log_trace(...) vsm_log(::vsm::log_level::trace, __VA_ARGS__)
#else
#	define vsm_log_trace(...) ((void)0)
#endif
