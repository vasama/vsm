#pragma once

#include <vsm/concepts.hpp>
#include <vsm/detail/function.hpp>
#include <vsm/standard.hpp>

#include <utility>

#include <cstddef>

namespace vsm {
namespace detail {

template<typename T>
void static_function_relocate(void* const object, void* const buffer, size_t const buffer_size)
{
	new (buffer) T(vsm_move(*reinterpret_cast<T*>(object)));
	reinterpret_cast<T*>(object)->~T();
}

inline void static_function_relocate_trivial(void* const object, void* const buffer, size_t const buffer_size)
{
	memcpy(buffer, object, buffer_size);
}

template<typename T>
void static_function_destroy(void* const object)
{
	reinterpret_cast<T*>(object)->~T();
}

inline void static_function_destroy_trivial(void* const object)
{
}

template<typename Result, typename... Params>
struct static_function_table
{
	Result(*invoke)(void* object, int category, Params... args);
	void(*relocate)(void* object, void* buffer, size_t buffer_size);
	void(*destroy)(void* object);

	template<typename T, int Category>
	static static_function_table const instance;
};

template<typename Result, typename... Params>
template<typename T, int Category>
static_function_table<Result, Params...> const static_function_table<Result, Params...>::instance =
{
	function_invoke_object<T, Category, Result, Params...>,
	std::is_trivially_copyable_v<T> ? static_function_relocate_trivial : static_function_relocate<T>,
	std::is_trivially_destructible_v<T> ? static_function_destroy_trivial : static_function_destroy<T>,
};

template<size_t Capacity>
struct static_function_buffer
{
	char buffer alignas(void*)[Capacity];

	constexpr static_function_buffer()
	{
		vsm_if_consteval
		{
			for (size_t i = 0; i < Capacity; ++i)
			{
				buffer[i] = 0;
			}
		}
	}
};

template<typename Signature, int Category, size_t Capacity>
class static_function_impl;

template<typename Result, typename... Params, int Category, size_t Capacity>
class static_function_impl<Result(Params...), Category, Capacity>
{
	using signature_type = Result(Params...);
	using table_type = static_function_table<Result, Params...>;

	table_type const* m_table;
	static_function_buffer<Capacity> m_buffer;

public:
	constexpr static_function_impl()
	{
		m_table = nullptr;
	}

	template<no_cvref_of<static_function_impl> Callable>
	constexpr static_function_impl(Callable&& callable)
		requires
			detail::callable<std::remove_cvref_t<Callable>, Category, Result, Params...> &&
			(sizeof(Callable) <= Capacity)
	{
		using callable_type = std::decay_t<Callable>;
		m_table = &table_type::template instance<callable_type, Category>;
		new (m_buffer.buffer) callable_type(vsm_forward(callable));
	}

	template<typename Callable>
	constexpr static_function_impl(std::in_place_type_t<Callable>) = delete;

	template<typename Callable, typename... Args>
	constexpr static_function_impl(std::in_place_type_t<Callable>, Args&&... args)
		requires
			detail::callable<std::remove_cvref_t<Callable>, Category, Result, Params...> &&
			(sizeof(Callable) <= Capacity)
	{
		m_table = &table_type::template instance<Callable, Category>;
		new (m_buffer.buffer) Callable(vsm_forward(args)...);
	}

	constexpr static_function_impl(static_function_impl&& src)
		: m_table(src.m_table)
	{
		if (src.m_table != nullptr)
		{
			src.m_table->relocate(src.m_buffer.buffer, m_buffer.buffer, sizeof(m_buffer.buffer));
		}

		src.m_table = nullptr;
	}

	template<size_t OtherCapacity, int OtherCategory>
	constexpr static_function_impl(static_function_impl<signature_type, OtherCategory, OtherCapacity>&& src)
		requires (OtherCapacity <= Capacity && (OtherCategory & Category) == Category)
		: m_table(src.m_table)
	{
		if (src.m_table != nullptr)
		{
			src.m_table->relocate(src.m_buffer.buffer, m_buffer.buffer, sizeof(m_buffer.buffer));
		}

		src.m_table = nullptr;
	}

	constexpr static_function_impl& operator=(static_function_impl&& src) &
	{
		if (m_table != nullptr)
		{
			m_table->destroy(m_buffer.buffer);
		}

		if (src.m_table != nullptr)
		{
			src.m_table->relocate(src.m_buffer.buffer, m_buffer.buffer, sizeof(m_buffer.buffer));
		}

		m_table = src.m_table;
		src.m_table = nullptr;

		return *this;
	}

	template<size_t OtherCapacity, int OtherCategory>
	constexpr static_function_impl& operator=(static_function_impl<signature_type, OtherCategory, OtherCapacity>&& src) &
		requires (OtherCapacity <= Capacity && (OtherCategory & Category) == Category)
	{
		if (m_table != nullptr)
		{
			m_table->destroy(m_buffer.buffer);
		}

		if (src.m_table != nullptr)
		{
			src.m_table->relocate(src.m_buffer.buffer, m_buffer.buffer, sizeof(m_buffer.buffer));
		}

		m_table = src.m_table;
		src.m_table = nullptr;

		return *this;
	}

	constexpr ~static_function_impl()
	{
		if (m_table != nullptr)
		{
			m_table->destroy(m_buffer.buffer);
		}
	}


	explicit constexpr operator bool() const
	{
		return m_table != nullptr;
	}


	Result operator()(std::convertible_to<Params> auto&&... args) &
		requires (Category & 0b1000)
	{
		vsm_assert(m_table != nullptr);
		return m_table->invoke(m_buffer.buffer, 0, vsm_forward(args)...);
	}

	Result operator()(std::convertible_to<Params> auto&&... args) &&
		requires (Category & 0b0100)
	{
		vsm_assert(m_table != nullptr);
		return m_table->invoke(m_buffer.buffer, 1, vsm_forward(args)...);
	}

	Result operator()(std::convertible_to<Params> auto&&... args) const&
		requires (Category & 0b0010)
	{
		vsm_assert(m_table != nullptr);
		return m_table->invoke(m_buffer.buffer, 2, vsm_forward(args)...);
	}

	Result operator()(std::convertible_to<Params> auto&&... args) const&&
		requires (Category & 0b0001)
	{
		vsm_assert(m_table != nullptr);
		return m_table->invoke(m_buffer.buffer, 3, vsm_forward(args)...);
	}
};

} // namespace detail

template<typename Signature, size_t Capacity = detail::function_default_capacity>
using static_function = detail::static_function_impl<
	typename detail::function_traits<Signature>::signature,
	detail::function_traits<Signature>::category,
	Capacity>;

} // namespace vsm
