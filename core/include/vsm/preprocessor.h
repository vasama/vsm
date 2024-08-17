#pragma once

#define vsm_pp_counter __COUNTER__

#define vsm_pp_empty(...)

#define vsm_pp_expand(...) __VA_ARGS__

#define vsm_detail_pp_cat(a, b) a ## b
#define vsm_pp_cat(a, b) vsm_detail_pp_cat(a, b)

#define vsm_detail_pp_str(...) #__VA_ARGS__
#define vsm_pp_str(...) vsm_detail_pp_str(__VA_ARGS__)

#define vsm_pp_anonymous(name) vsm_pp_cat(name, vsm_pp_counter)

#define vsm_pp_include(...) <__VA_ARGS__>

#define vsm_detail_pp_x_size(...) +1
#define vsm_pp_x_size(x_macro) (x_macro(vsm_detail_pp_x_size))

#if 0
#define vsm_detail_pp_x_comma2(discard, ...) __VA_ARGS__
#define vsm_detail_pp_x_comma1(...) , __VA_ARGS__

#define vsm_pp_x_comma(x_macro) \
	vsm_pp_expand(vsm_pp_detail_x_comma2 vsm_pp_expand((x_macro(vsm_pp_detail_x_comma1))))
#endif
