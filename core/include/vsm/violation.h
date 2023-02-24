#pragma once

#include <vsm/platform.h>

#if __cplusplus
extern "C" {
#endif

[[noreturn]] void vsm_violation_handler();

#if vsm_config_harden
#	define vsm_violation() vsm_violation_handler
#else
#	define vsm_violation()
#endif

#if __cplusplus
} // extern "C"
#endif
