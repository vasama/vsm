#pragma once

#define vsm_detail_reference_categories(X, ...) \
	X(&             __VA_OPT__(, __VA_ARGS__)) \
	X(const&        __VA_OPT__(, __VA_ARGS__)) \
	X(&&            __VA_OPT__(, __VA_ARGS__)) \
	X(const&&       __VA_OPT__(, __VA_ARGS__)) \
