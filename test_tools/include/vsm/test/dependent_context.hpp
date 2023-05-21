#pragma once

#include <type_traits>

namespace vsm::detail {

template<typename T>
using dependent_type_identity = T;

template<bool>
struct dependent_value;

template<>
struct dependent_value<0>
{
	template<typename T>
	static T identity(T const&);
};

template<>
struct dependent_value<1>
{
	template<typename T>
	static T&& identity(T&&);
};

template<bool>
struct dependent_impl
{
	template<typename T>
	using type = T;

	template<bool Reference>
	using value = dependent_value<Reference>;
};

#define vsm_dependent_context(...) \
	static_assert(([]<bool vsm_detail_dependent_context = true>() -> void { __VA_ARGS__ }(), true))

#define vsm_dep_t(...) \
	::vsm::detail::dependent_type_identity<typename ::vsm::detail::dependent_impl<vsm_detail_dependent_context>::template type<__VA_ARGS__>>

#define vsm_dep_v(...) \
	::vsm::detail::dependent_impl<vsm_detail_dependent_context>::template value<::std::is_reference_v<decltype((__VA_ARGS__))>>::identity(__VA_ARGS__)

} // namespace vsm::detail
