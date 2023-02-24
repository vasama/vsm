#pragma once

#include <vsm/platform.h>
#include <vsm/utility.hpp>

#include <bit>

namespace vsm::detail {

template<typename Signature>
struct function_traits;

template<typename Result, typename... Params>
struct function_traits<Result(Params...)>
{
	using signature = Result(Params...);
	static constexpr int category = 0b1111;
};

template<typename Result, typename... Params>
struct function_traits<Result(Params...) const>
{
	using signature = Result(Params...);
	static constexpr int category = 0b0011;
};

template<typename Result, typename... Params>
struct function_traits<Result(Params...) &>
{
	using signature = Result(Params...);
	static constexpr int category = 0b1000;
};

template<typename Result, typename... Params>
struct function_traits<Result(Params...) &&>
{
	using signature = Result(Params...);
	static constexpr int category = 0b0100;
};

template<typename Result, typename... Params>
struct function_traits<Result(Params...) const&>
{
	using signature = Result(Params...);
	static constexpr int category = 0b0010;
};

template<typename Result, typename... Params>
struct function_traits<Result(Params...) const&&>
{
	using signature = Result(Params...);
	static constexpr int category = 0b0001;
};

template<typename T, int Category, typename Result, typename... Params>
concept callable =
	((Category & 0b1000) == 0 || std::is_convertible_v<std::invoke_result_t<T&, Params...>, Result>) &&
	((Category & 0b0100) == 0 || std::is_convertible_v<std::invoke_result_t<T&&, Params...>, Result>) &&
	((Category & 0b0010) == 0 || std::is_convertible_v<std::invoke_result_t<T const&, Params...>, Result>) &&
	((Category & 0b0001) == 0 || std::is_convertible_v<std::invoke_result_t<T const&&, Params...>, Result>);

template<typename Result, typename... Params>
union function_context
{
	void const* object;
	Result(*function)(Params...);

	constexpr function_context() = default;

	constexpr function_context(void const* const object)
		: object(object)
	{
	}

	constexpr function_context(Result(*function)(Params...))
		: function(function)
	{
	}
};

class function_category_0
{
public:
	function_category_0() = default;

	consteval function_category_0(int)
	{
	}
	
	static constexpr bool equals(int)
	{
		return true;
	}
};

template<int Category>
class function_category_1
{
	int m_value;

public:
	consteval function_category_1(int const value)
		: m_value(value)
	{
	}

	constexpr bool equals(int const value)
	{
		return m_value == value;
	}
};

template<bool>
struct function_category_select;

template<>
struct function_category_select<0>
{
	template<int Category>
	using type = function_category_0;
};

template<>
struct function_category_select<1>
{
	template<int Category>
	using type = function_category_1<Category>;
};

template<int Category>
using function_category = typename function_category_select<(Category & Category - 1)>::template type<Category>;

template<typename T, int Category, typename Result, typename... Params>
Result function_invoke_object(
	function_context<Result, Params...> const context,
	function_category<Category> const category, Params... args)
{
	static_assert(Category & 0b1111);

	if constexpr (Category & 0b1000)
	{
		if (category.value == 0)
		{
			return static_cast<T&>(*static_cast<T*>(const_cast<void*>(context.object)))(vsm_move(args)...);
		}
	}

	if constexpr (Category & 0b0100)
	{
		if (category.value == 1)
		{
			return static_cast<T&&>(*static_cast<T*>(const_cast<void*>(context.object)))(vsm_move(args)...);
		}
	}

	if constexpr (Category & 0b0010)
	{
		if (category.value == 2)
		{
			return static_cast<T const&>(*static_cast<T const*>(context.object))(vsm_move(args)...);
		}
	}

	if constexpr (Category & 0b0001)
	{
		if (category.value == 3)
		{
			return static_cast<T const&&>(*static_cast<T const*>(context.object))(vsm_move(args)...);
		}
	}

	vsm_unreachable();
}

template<int Category, typename Result, typename... Params>
Result function_invoke_pointer(
	function_context<Result, Params...> const context,
	function_category<Category> const category, Params... args)
{
	return context.function(vsm_move(args)...);
}

inline constexpr size_t function_default_capacity = 2 * sizeof(void*);

} // namespace vsm::detail
