#pragma once

#include <vsm/result.hpp>

#include <concepts>

namespace vsm {
namespace detail {

template<typename T>
concept _try_for_boolean_testable = requires (T&& t)
{
	static_cast<bool>(static_cast<T&&>(t));
};

//TODO: Move to vsm/concepts.hpp
template<typename T>
concept try_for_boolean_testable =
	_try_for_boolean_testable<T> &&
	requires (T&& t)
	{
		{ !static_cast<T&&>(t) } -> _try_for_boolean_testable;
	};

template<typename Result>
concept try_for_result =
	try_for_boolean_testable<Result const&> &&
	requires (Result&& result)
	{
		vsm::propagate_error(static_cast<Result&&>(result));
		{ *static_cast<Result&&>(result) } -> try_for_boolean_testable;
	};

template<typename Iterator>
concept try_for_iterator = requires (Iterator& iterator)
{
	{ iterator.next() } -> try_for_result;
	iterator.get();
};

} // namespace detail

#define vsm_detail_try_for(spec, iterator, result, ...) \
	for (::vsm::detail::try_for_iterator decltype(auto) iterator = (__VA_ARGS__);;) \
		if (auto result = iterator.next(); !vsm_as_const(result)) \
		{ \
			return ::vsm::propagate_error(vsm_move(result)); \
		} \
		else if (!*vsm_move(result)) \
		{ \
			break; \
		} \
		else if (spec = iterator.get(); false) \
		{ \
		} \
		else \

#define vsm_try_for(spec, ...) \
	vsm_detail_try_for( \
		spec, \
		vsm_pp_anonymous(vsm_detail_try_for_i), \
		vsm_pp_anonymous(vsm_detail_try_for_r), \
		__VA_ARGS__)

} // namespace vsm
