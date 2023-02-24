#pragma once

#include <vsm/assert.h>
#include <vsm/concepts.hpp>
#include <vsm/detail/function.hpp>
#include <vsm/standard.hpp>

namespace vsm {
namespace detail {

template<typename T>
std::integral_constant<int, 0b1000> function_category_detect_1(T&);

template<typename T>
std::integral_constant<int, 0b0100> function_category_detect_1(T&&);

template<typename T>
std::integral_constant<int, 0b0010> function_category_detect_1(T const&);

template<typename T>
std::integral_constant<int, 0b0001> function_category_detect_1(T const&&);

template<typename T>
inline constexpr int function_category_detect = decltype(
	function_category_detect_1<std::remove_cvref_t<T>>(std::declval<T>()))::value;

template<typename Signature, int Category>
class function_view_impl;

template<typename Result, typename... Params, int Category>
class function_view_impl<Result(Params...), Category>
{
	using signature_type = Result(Params...);
	using context_type = function_context<Result, Params...>;

	Result(*m_invoke)(context_type context, function_category_0, Params... args);
	context_type m_context;

public:
	constexpr function_view_impl()
		: m_invoke(nullptr)
	{
		vsm_if_consteval
		{
			m_context.object = nullptr;
		}
	}

	template<no_cvref_of<function_view_impl> Callable,
		int CallableCategory = function_category_detect<Callable&&>>
	constexpr function_view_impl(Callable&& callable)
		requires detail::callable<std::remove_cvref_t<Callable>, Category, Result, Params...>
		: m_invoke(function_invoke_object<std::decay_t<Callable>, CallableCategory, Result, Params...>)
		, m_context(&callable)
	{
	}

	constexpr function_view_impl(signature_type& function)
		: m_invoke(function_invoke_pointer<Category, Result, Params...>)
		, m_context(&function)
	{
	}

	// Deleted because borrowing a function pointer is likely not intended.
	// Dereference the function pointer to instead bind directly to the function.
	template<typename Signature>
	constexpr function_view_impl(Signature*)
		requires std::is_function_v<Signature> = delete;

	template<int OtherCategory>
	constexpr function_view_impl(function_view_impl<signature_type, OtherCategory> const& src)
		requires ((OtherCategory & Category) == Category)
		: m_invoke(src.m_invoke)
		, m_context(src.m_context)
	{
	}

	function_view_impl(function_view_impl const&) = default;
	function_view_impl& operator=(function_view_impl const&) & = default;


	explicit constexpr operator bool() const
	{
		return m_invoke != nullptr;
	}


	Result operator()(std::convertible_to<Params> auto&&... args) const
	{
		vsm_assert(m_invoke != nullptr);
		return m_invoke(m_context, {}, vsm_forward(args)...);
	}
};

} // namespace detail

template<typename Signature>
using function_view = detail::function_view_impl<
	typename detail::function_traits<Signature>::signature,
	detail::function_traits<Signature>::category>;

} // namespace vsm
