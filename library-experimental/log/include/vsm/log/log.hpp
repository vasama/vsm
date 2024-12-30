#pragma once

#include <vsm/log/level.hpp>
#include <vsm/platform.h>
#include <vsm/standard.hpp>
#include <vsm/utility.hpp>

#include <source_location>

namespace vsm {

template<typename Logger>
concept logger = requires
{
	//typename Logger::log_scope_type;
	typename Logger::log_entry_type;
	Logger::log_entry_type(
		//std::declval<typename Logger::log_scope_type const&>(),
		"",
		std::declval<std::source_location const&>());
	{ std::declval<Logger const&>().get_level() } noexcept -> std::same_as<log_level>;
};

namespace detail {

template<typename Logger>
concept _cvref_logger = logger<std::remove_cvref_t<Logger>>;

} // namespace detail

#if vsm_compiler_preview
#	define vsm_detail_log_logger_concept
#else
#	define vsm_detail_log_logger_concept ::vsm::detail::_cvref_logger
#endif

//TODO: Validate the format string and arguments.
// The _ macro prefixes internal names to avoid collisions.
#define vsm_detail_log(_, logger_arg, level_arg, message_arg, ...) \
	([&] vsm_lambda_attribute(vsm_always_inline) () -> void \
	{ \
		vsm_detail_log_logger_concept auto&& _(logger) = (logger_arg); \
		using _(logger_t) = ::std::remove_reference_t<decltype(_(logger))>; \
		static constexpr ::vsm::log_level _(level) = (level_arg); \
		if (_(level) <= static_cast<_(logger_t) const&>(_(logger)).get_level()) \
		{ \
			using _(entry_t) = typename _(logger_t)::log_entry_type; \
			static constexpr _(entry_t) _(entry)( \
				_(level), \
				::std::source_location::current(), \
				message_arg); \
			static_cast<_(logger_t)&&>(_(logger)).log( \
				_(entry) \
				__VA_OPT__(, __VA_ARGS__)); \
		} \
	}())

#define vsm_detail_log_name(name) \
	vsm_detail_log_ ## name

#define vsm_log(logger, level, message, ...) \
	vsm_detail_log(vsm_detail_log_name, logger, level, message __VA_OPT__(, __VA_ARGS__))

#if vsm_config_log_level >= vsm_log_level_fatal
#	define vsm_log_fatal(logger, message, ...) \
		vsm_log(logger, ::vsm::log_level::fatal, message __VA_OPT__(, __VA_ARGS__))
#else
#	define vsm_log_fatal(logger, message, ...) ((void)0)
#endif

#if vsm_config_log_level >= vsm_log_level_error
#	define vsm_log_error(logger, message, ...) \
		vsm_log(logger, ::vsm::log_level::error, message __VA_OPT__(, __VA_ARGS__))
#else
#	define vsm_log_error(logger, message, ...) ((void)0)
#endif

#if vsm_config_log_level >= vsm_log_level_warn
#	define vsm_log_warn(logger, message, ...) \
		vsm_log(logger, ::vsm::log_level::warn, message __VA_OPT__(, __VA_ARGS__))
#else
#	define vsm_log_warn(logger, message, ...) ((void)0)
#endif

#if vsm_config_log_level >= vsm_log_level_info
#	define vsm_log_info(logger, message, ...) \
		vsm_log(logger, ::vsm::log_level::info, message __VA_OPT__(, __VA_ARGS__))
#else
#	define vsm_log_info(logger, message, ...) ((void)0)
#endif

#if vsm_config_log_level >= vsm_log_level_debug
#	define vsm_log_debug(logger, message, ...) \
		vsm_log(logger, ::vsm::log_level::debug, message __VA_OPT__(, __VA_ARGS__))
#else
#	define vsm_log_debug(logger, message, ...) ((void)0)
#endif

#if vsm_config_log_level >= vsm_log_level_trace
#	define vsm_log_trace(logger, message, ...) \
		vsm_log(logger, ::vsm::log_level::trace, message __VA_OPT__(, __VA_ARGS__))
#else
#	define vsm_log_trace(logger, message, ...) ((void)0)
#endif

} // namespace vsm
