#pragma once

namespace vsm {

#define vsm_log_level_none              0
#define vsm_log_level_fatal             1
#define vsm_log_level_error             2
#define vsm_log_level_warn              3
#define vsm_log_level_info              4
#define vsm_log_level_debug             5
#define vsm_log_level_trace             6

#ifndef vsm_config_log_level
#	define vsm_config_log_level vsm_log_level_trace
#endif

#define vsm_log_levels(X, ...) \
	X(none      __VA_OPT__(, __VA_ARGS__)) \
	X(fatal     __VA_OPT__(, __VA_ARGS__)) \
	X(error     __VA_OPT__(, __VA_ARGS__)) \
	X(warn      __VA_OPT__(, __VA_ARGS__)) \
	X(info      __VA_OPT__(, __VA_ARGS__)) \
	X(debug     __VA_OPT__(, __VA_ARGS__)) \
	X(trace     __VA_OPT__(, __VA_ARGS__)) \

enum class log_level : uint8_t
{
#define vsm_xentry(level) \
	level,

	vsm_log_levels(vsm_xentry)
#undef vsm_xentry
};

} // namespace vsm
