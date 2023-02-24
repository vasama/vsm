#pragma once

#include <vsm/any_ref.hpp>
#include <vsm/relocate.hpp>

#include <new>

namespace vsm {

template<size_t Capacity, typename... Functions>
class small_any;

namespace detail {

template<size_t Capacity, typename... Functions>
void _detect_small_any(small_any<Capacity, Functions...>);

template<typename T>
concept _not_small_any = !requires
{
	detail::_detect_small_any(std::declval<T>());
};

struct _any_relocate
{
	using signature_type = void(void* storage) noexcept;

	template<typename T>
	static void invoke(T& object, void* const storage) noexcept
	{
		relocate_at(std::addressof(object), reinterpret_cast<T*>(storage));
	}
};

struct _any_destroy
{
	using signature_type = void() noexcept;

	template<typename T>
	static void invoke(T& object) noexcept
	{
		object.~T();
	}
};

} // namespace detail

template<size_t Capacity, typename... Functions>
class small_any
{
	template<typename... UserFunctions>
	using functions_template = detail::_any_functions<
		detail::_any_relocate,
		detail::_any_destroy,
		UserFunctions...>;

	using functions_type = functions_template<Functions...>;

	detail::_any_function* const* m_functions;
	struct
	{
		alignas(std::max_align_t) unsigned char data[Capacity];
	}
	m_storage;

public:
	constexpr small_any()
		: m_functions(nullptr)
	{
		vsm_if_consteval
		{
			m_storage = {};
		}
	}

	template<no_cvref_of<small_any> T>
	small_any(T&& object)
		requires
			detail::_not_small_any<remove_cvref_t<T>> &&
			functions_type::template requirement<T&&> &&
			std::is_nothrow_move_constructible_v<std::decay_t<T>>
		: m_functions(functions_type::template table<std::decay_t<T>>)
	{
		new (m_storage.data) std::decay_t<T>(vsm_forward(object));
	}

	template<non_cvref T, typename... Args>
	small_any(std::in_place_type_t<T>, Args&&... args)
		requires
			detail::_not_small_any<T> &&
			functions_type::template requirement<T> &&
			std::is_nothrow_move_constructible_v<T>
		: m_functions(functions_type::template table<T>)
	{
		new (m_storage.data) T(vsm_forward(args)...);
	}

	template<size_t OtherCapacity, typename... ExtraFunctions>
	constexpr small_any(small_any<OtherCapacity, Functions..., ExtraFunctions...>&& other)
		requires (OtherCapacity >= Capacity && sizeof...(ExtraFunctions) > 0)
		: m_functions(other.m_functions)
	{
		if (other.m_functions != nullptr)
		{
			other.invoke<detail::_any_relocate>(m_storage.data);
			other.m_functions = nullptr;
		}
	}

	small_any(small_any&& other) noexcept
		: m_functions(other.m_functions)
	{
		if (other.m_functions != nullptr)
		{
			other.invoke<detail::_any_relocate>(m_storage.data);
			other.m_functions = nullptr;
		}
	}

	template<size_t OtherCapacity>
	small_any(small_any<OtherCapacity, Functions...>&& other) noexcept
		requires (OtherCapacity > Capacity)
		: m_functions(other.m_functions)
	{
		if (other.m_functions != nullptr)
		{
			other.invoke<detail::_any_relocate>(m_storage.data);
			other.m_functions = nullptr;
		}
	}

	small_any& operator=(small_any&& other) & noexcept
	{
		if (vsm_likely(this != &other))
		{
			if (m_functions != nullptr)
			{
				invoke<detail::_any_destroy>();
			}

			other.invoke<detail::_any_relocate>(m_storage.data);
			m_functions = other.m_functions;
			other.m_functions = nullptr;
		}
		return *this;
	}

	template<size_t OtherCapacity>
	small_any& operator=(small_any<OtherCapacity, Functions...>&& other) & noexcept
		requires (OtherCapacity > Capacity)
	{
		// Self assignment is not possible when the types are different.

		if (m_functions != nullptr)
		{
			invoke<detail::_any_destroy>();
		}

		other.invoke<detail::_any_relocate>(m_storage.data);
		m_functions = other.m_functions;
		other.m_functions = nullptr;

		return *this;
	}

	~small_any()
	{
		if (m_functions != nullptr)
		{
			invoke<detail::_any_destroy>();
		}
	}

	[[nodiscard]] explicit operator bool() const
	{
		return m_functions != nullptr;
	}

	template<typename F, typename Self, typename... Args>
	[[nodiscard]] auto invoke(this Self&& self, Args&&... args)
		requires detail::_any_invocable<F, Self, Args...>
	{
		auto const function = m_functions[pack_index<F, Functions...>];
		using function_type = typename detail::_any_traits_for<F>::function_type;
		return reinterpret_cast<function_type*>(function)(m_storage.data, vsm_forward(args)...);
	}

private:
	template<size_t, typename...>
	friend class small_any;
};

template<typename... Functions>
using any = small_any<sizeof(void*), Functions...>;

} // namespace vsm
