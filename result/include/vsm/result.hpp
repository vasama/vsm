#pragma once

#include <vsm/detail/expected.hpp>
#include <vsm/preprocessor.h>
#include <vsm/type_traits.hpp>

#include <system_error>

namespace vsm {

using detail::expected_namespace::expected;
using detail::expected_namespace::unexpected;

template<typename T, typename E = std::error_code>
using result = detail::expected_namespace::expected<T, E>;

inline constexpr auto const& result_value = detail::expected_namespace::in_place;
inline constexpr auto const& result_error = detail::expected_namespace::unexpect;

inline result<void> as_result(std::error_code const error) noexcept
{
	return error
		? result<void>{ result_error, error }
		: result<void>{};
}

inline std::error_code as_error_code(result<void> const& result) noexcept
{
	return result
		? std::error_code()
		: result.error();
}

template<typename T, typename E>
result<void, E> discard_value(result<T, E> const& r)
{
	return r ? result<void, E>{} : result<void, E>(r.error());
}


/// @brief Evaluate an expression and test the result.
///        Returns from the current subroutine if the result contains an error.
/// @param spec Name and optionally the type of the introduced variable.
/// @param ... Expression to evaluate.
#define vsm_try_result(spec, ...) \
	vsm_detail_try_result_1(return, spec, __VA_ARGS__)

/// @brief Evaluate an expression and test the result.
///        Returns from the current coroutine if the result contains an error.
/// @param spec Name and optionally the type of the introduced variable.
/// @param ... Expression to evaluate.
#define vsm_co_try_result(spec, ...) \
	vsm_detail_try_result_1(co_return, spec, __VA_ARGS__)


/// @brief Evaluate an expression and unwrap its non-void result into a new variable.
///        Returns from the current subroutine if the result contains an error.
/// @param spec Name and optionally the type of the introduced variable.
/// @param ... Expression to evaluate.
#define vsm_try(spec, ...) \
	vsm_detail_try_1(return, spec, __VA_ARGS__)

/// @brief Evaluate an expression and unwrap its non-void result into a new variable.
///        Returns from the current coroutine if the result contains an error.
/// @param spec Name and optionally the type of the introduced variable.
/// @param ... Expression to evaluate.
#define vsm_co_try(spec, ...) \
	vsm_detail_try_1(co_return, spec, __VA_ARGS__)


/// @brief Evaluate an expression and unwrap its void result.
///        Returns from the current subroutine if the result contains an error.
/// @param ... Expression to evaluate.
#define vsm_try_void(...) \
	vsm_detail_try_void_1(return, __VA_ARGS__)

/// @brief Evaluate an expression and unwrap its void result.
///        Returns from the current coroutine if the result contains an error.
/// @param ... Expression to evaluate.
#define vsm_co_try_void(...) \
	vsm_detail_try_void_1(co_return, __VA_ARGS__)


/// @brief Evaluate an expression and unwrap its result discarding it.
///        Returns from the current subroutine if the result contains an error.
/// @param ... Expression to evaluate.
#define vsm_try_discard(...) \
	vsm_detail_try_discard_1(return, __VA_ARGS__)

/// @brief Evaluate an expression and unwrap its result discarding it.
///        Returns from the current coroutine if the result contains an error.
/// @param ... Expression to evaluate.
#define vsm_co_try_discard(...) \
	vsm_detail_try_discard_1(co_return, __VA_ARGS__)


/// @brief Evaluate an expression and unwrap its non-void result into new variables in a structured binding.
///        Returns from the current subroutine if the result contains an error.
/// @param spec List of names introduced by the structured binding.
/// @param ... Expression to evaluate.
#define vsm_try_bind(spec, ...) \
	vsm_detail_try_bind_1(return, spec, __VA_ARGS__)

/// @brief Evaluate an expression and unwrap its non-void result into new variables in a structured binding.
///        Returns from the current coroutine if the result contains an error.
/// @param spec List of names introduced by the structured binding.
/// @param ... Expression to evaluate.
#define vsm_co_try_bind(spec, ...) \
	vsm_detail_try_bind_1(co_return, spec, __VA_ARGS__)


/// @brief Evaluate an expression and unwrap its non-void result assigning it to the specified lvalue expression.
///        Returns from the current subroutine if the result contains an error.
/// @param left Mutable lvalue expressiont to which the result is assigned.
/// @param ... Expression to evaluate.
#define vsm_try_assign(left, ...) \
	vsm_detail_try_assign_1(return, left, __VA_ARGS__)

/// @brief Evaluate an expression and unwrap its non-void result assigning it to the specified lvalue expression.
///        Returns from the current coroutine if the result contains an error.
/// @param left Mutable lvalue expressiont to which the result is assigned.
/// @param ... Expression to evaluate.
#define vsm_co_try_assign(left, ...) \
	vsm_detail_try_assign_1(co_return, left, __VA_ARGS__)


/* Try-macro implementation details. */

#define vsm_detail_try_is_void(result) \
	(::std::is_void_v<typename ::std::remove_cvref_t<decltype(result)>::value_type>)

#define vsm_detail_try_s0(_0, ...) \
	_0

#define vsm_detail_try_s2(_0, _1, _2, ...) \
	_2

#define vsm_detail_try_h(spec) \
	vsm_pp_expand(vsm_detail_try_s0 vsm_pp_expand(vsm_detail_try_s2 vsm_pp_expand((vsm_pp_expand spec, spec, (auto)))))

#define vsm_detail_try_v(spec) \
	vsm_pp_expand(vsm_detail_try_s2 vsm_pp_expand((, vsm_pp_expand spec, spec)))

#define vsm_detail_try_r \
	vsm_pp_cat(vsm_detail_try_r, __COUNTER__)

#define vsm_detail_try_unlikely

#define vsm_detail_try_intro(return, hspec, result, ...) \
	hspec result = (__VA_ARGS__); \
	if (!result) \
	{ \
		vsm_detail_try_unlikely \
		return ::vsm::unexpected(static_cast<decltype(result)&&>(result).error()); \
	}

/* vsm_try_result */

#define vsm_detail_try_result_2(return, hspec, result, ...) \
	vsm_detail_try_intro(return, hspec, result, __VA_ARGS__) \
	((void)0)

#define vsm_detail_try_result_1(return, spec, ...) \
	vsm_detail_try_result_2(return, vsm_detail_try_h(spec), vsm_detail_try_v(spec), __VA_ARGS__)


/* vsm_try */

#define vsm_detail_try_2(return, hspec, vspec, result, ...) \
	vsm_detail_try_intro(return, hspec, result, __VA_ARGS__) \
	static_assert(!vsm_detail_try_is_void(result), "try requires a non-void result."); \
	auto&& vspec = *static_cast<decltype(result)&&>(result); \
	((void)0)

#define vsm_detail_try_1(return, spec, ...) \
	vsm_detail_try_2(return, vsm_detail_try_h(spec), vsm_detail_try_v(spec), vsm_detail_try_r, __VA_ARGS__)

/* vsm_try_void */

#define vsm_detail_try_void_2(return, hspec, result, ...) \
	{ \
		vsm_detail_try_intro(return, hspec, result, __VA_ARGS__) \
		static_assert(vsm_detail_try_is_void(result), "try_void requires a void result."); \
	} \
	((void)0)

#define vsm_detail_try_void_1(return, ...) \
	vsm_detail_try_void_2(return, auto, vsm_detail_try_r, __VA_ARGS__)

/* vsm_try_discard */

#define vsm_detail_try_discard_2(return, hspec, result, ...) \
	{ \
		vsm_detail_try_intro(return, hspec, result, __VA_ARGS__) \
	} \
	((void)0)

#define vsm_detail_try_discard_1(return, ...) \
	vsm_detail_try_discard_2(return, auto, vsm_detail_try_r, __VA_ARGS__)

/* vsm_try_bind */

#define vsm_detail_try_bind_2(return, hspec, vspec, result, ...) \
	vsm_detail_try_intro(return, hspec, result, __VA_ARGS__) \
	static_assert(!vsm_detail_try_is_void(result), "try_bind requires a non-void result."); \
	auto&& [vsm_pp_expand vspec] = *static_cast<decltype(result)&&>(result); \
	((void)0)

#define vsm_detail_try_bind_1(return, spec, ...) \
	vsm_detail_try_bind_2(return, auto, spec, vsm_detail_try_r, __VA_ARGS__)

/* vsm_try_assign */

#define vsm_detail_try_assign_2(return, hspec, left, result, ...) \
	{ \
		vsm_detail_try_intro(return, hspec, result, __VA_ARGS__) \
		static_assert(!vsm_detail_try_is_void(result), "try_assign requires a non-void result."); \
		((void)(left = *static_cast<decltype(result)&&>(result))); \
	} \
	((void)0)

#define vsm_detail_try_assign_1(return, left, ...) \
	vsm_detail_try_assign_2(return, vsm_detail_try_h(left), vsm_detail_try_v(left), vsm_detail_try_r, __VA_ARGS__)

} // namespace vsm
