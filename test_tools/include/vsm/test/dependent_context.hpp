#pragma once

#include <vsm/preprocessor.h>

#include <type_traits>

namespace vsm::detail {

struct dependent_context
{
	template<bool>
	struct select;

	template<>
	struct select<0>
	{
		template<typename T>
		static T identity(T);
	};

	template<>
	struct select<1> : std::true_type
	{
		template<typename T>
		using type = T;

		template<typename T>
		static T&& identity(T&&);
	};

	dependent_context(auto lambda)
	{
		static_assert(select<std::is_void_v<decltype(lambda())>>::value);
	}

	template<bool Bool, typename T>
	using type = typename select<Bool>::template type<T>;

	template<bool Bool, typename T>
	using value = select<Bool && std::is_reference_v<T>>;
};

#define vsm_dependent_context \
	static ::vsm::detail::dependent_context const \
	vsm_pp_cat(vsm_detail_dependent_context, vsm_pp_counter) = \
		[]<bool vsm_detail_dependent_context = true>() -> auto

#define vsm_dep_t(...) \
	::vsm::detail::dependent_context::type<vsm_detail_dependent_context, __VA_ARGS__>

#define vsm_dep_v(...) \
	(::vsm::detail::dependent_context::value<vsm_detail_dependent_context, decltype((__VA_ARGS__))>::identity(__VA_ARGS__))

} // namespace vsm::detail
