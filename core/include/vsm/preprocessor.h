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
