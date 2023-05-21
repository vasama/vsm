#pragma once

#include <vsm/tag_invoke.hpp>
#include <vsm/utility.hpp>

#include <type_traits>

namespace vsm {
namespace detail {

template<typename>
struct inspect_helper;

template<typename T>
struct inspect_helper<void(T)>
{
	using type = T;
};

} // namespace detail

struct inspect_cpo
{
};

struct match_index_cpo
{
};

struct match_value_cpo
{
};

#define vsm_inspect(...) \
	if (auto&& vsm_detail_inspect_v = (__VA_ARGS__); true) \
	if (typedef ::std::remove_cvref_t<decltype(vsm_detail_inspect_v)> vsm_detail_inspect_t; true) \
	switch (::vsm::tag_invoke(::vsm::inspect_cpo(), vsm_as_const(vsm_detail_inspect_v)))

#define vsm_match(...) \
	break; \
	if (typedef typename ::vsm::detail::inspect_helper<void(__VA_ARGS__)>::type vsm_detail_match_t; true) \
	if (typedef ::std::integral_constant<size_t, ::vsm::tag_invoke( \
		::vsm::match_index_cpo(), ::vsm::tag<vsm_detail_inspect_t>(), ::vsm::tag<vsm_detail_match_t>())> vsm_detail_match_i; true) \
	case vsm_detail_match_i::value: \
	if (__VA_ARGS__ = ::vsm::tag_invoke( \
		::vsm::match_value_cpo(), vsm_detail_inspect_v, ::vsm::tag<vsm_detail_match_t>()); true)

#define vsm_match_default \
	break; \
	default: if (true)

} // namespace vsm
