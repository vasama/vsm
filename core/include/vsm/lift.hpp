#pragma once

#include <vsm/utility.hpp>

#define vsm_detail_lift(capture, ...) ( \
		[capture](auto&&... args) \
			noexcept(noexcept(__VA_ARGS__(vsm_forward(args)...))) \
			-> decltype(__VA_ARGS__(vsm_forward(args)...)) \
		{ return __VA_ARGS__(vsm_forward(args)...); } \
	)

#define vsm_lift(...) vsm_detail_lift(, __VA_ARGS__)
#define vsm_lift_t(...) decltype(vsm_lift(__VA_ARGS__))
#define vsm_lift_copy(...) vsm_detail_lift(=, __VA_ARGS__)
#define vsm_lift_borrow(...) vsm_detail_lift(&, __VA_ARGS__)


#define vsm_detail_bind(capture, function, ...) ( \
		[capture](auto&&... args) \
			noexcept(noexcept(function(__VA_ARGS__, vsm_forward(args)...))) \
			-> decltype(function(__VA_ARGS__, vsm_forward(args)...)) \
		{ return function(__VA_ARGS__, vsm_forward(args)...); } \
	)

#define vsm_bind(function, ...) vsm_detail_bind(, function, __VA_ARGS__)
#define vsm_bind_copy(function, ...) vsm_detail_bind(=, function, __VA_ARGS__)
#define vsm_bind_borrow(function, ...) vsm_detail_bind(&, function, __VA_ARGS__)
