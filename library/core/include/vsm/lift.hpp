#pragma once

#include <vsm/utility.hpp>

#define vsm_detail_lift_wrap(capture, ...) ( \
		[capture](auto&&... vsm_detail_args) \
			noexcept(noexcept(__VA_ARGS__)) \
			-> decltype(__VA_ARGS__) \
		{ return (__VA_ARGS__); } \
	)


#define vsm_detail_lift(capture, ...) \
	vsm_detail_lift_wrap(capture, __VA_ARGS__(vsm_forward(vsm_detail_args)...))

#define vsm_lift(...) vsm_detail_lift(, __VA_ARGS__)
#define vsm_lift_t(...) decltype(vsm_lift(__VA_ARGS__))
#define vsm_lift_copy(...) vsm_detail_lift(=, __VA_ARGS__)
#define vsm_lift_borrow(...) vsm_detail_lift(&, __VA_ARGS__)


#define vsm_detail_bind_front(capture, function, ...) \
	vsm_detail_lift_wrap(capture, function(__VA_OPT__(__VA_ARGS__,) vsm_forward(vsm_detail_args)...))

#define vsm_bind_front(function, ...) vsm_detail_bind_front(, function, __VA_ARGS__)
#define vsm_bind_front_t(function, ...) decltype(vsm_bind_front(function, __VA_ARGS__))
#define vsm_bind_front_copy(function, ...) vsm_detail_bind_front(=, function, __VA_ARGS__)
#define vsm_bind_front_borrow(function, ...) vsm_detail_bind_front(&, function, __VA_ARGS__)


#define vsm_detail_bind_back(capture, function, ...) \
	vsm_detail_lift_wrap(capture, function(vsm_forward(vsm_detail_args)... __VA_OPT__(, __VA_ARGS__)))

#define vsm_bind_back(function, ...) vsm_detail_bind_back(, function, __VA_ARGS__)
#define vsm_bind_back_t(function, ...) decltype(vsm_bind_back(function, __VA_ARGS__))
#define vsm_bind_back_copy(function, ...) vsm_detail_bind_back(=, function, __VA_ARGS__)
#define vsm_bind_back_borrow(function, ...) vsm_detail_bind_back(&, function, __VA_ARGS__)
