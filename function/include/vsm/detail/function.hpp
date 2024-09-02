#pragma once

#include <vsm/assert.h>
#include <vsm/concepts.hpp>
#include <vsm/standard.hpp>
#include <vsm/utility.hpp>

#include <new>
#include <utility>

#include <cstddef>
#include <cstring>

namespace vsm::detail {

template<typename T, bool N, typename R, typename... Ps>
R _function_invoke(void* const object, Ps... args) noexcept(N)
{
	static_assert(std::is_reference_v<T>);
	return static_cast<T>(*std::launder(static_cast<remove_cvref_t<T>*>(object)))(vsm_move(args)...);
}

template<typename T>
void _function_relocate(void* const src, void* const dst)
{
	if (src != dst)
	{
		T* const ptr = std::launder(reinterpret_cast<T*>(src));
		new (dst) T(vsm_move(*ptr));
		ptr->~T();
	}
}

template<typename T>
void _function_destroy(void* const object)
{
	std::launder(reinterpret_cast<T*>(object))->~T();
}

template<size_t Size>
void _function_relocate_trivial(void* const src, void* const dst)
{
	if (src != dst)
	{
		memcpy(dst, src, Size);
	}
}

inline void _function_destroy_trivial(void*)
{
}

//TODO: Deal with this warning
vsm_msvc_warning(push)
//vsm_msvc_warning(disable: 4268)

template<bool N, typename R, typename... Ps>
struct _function_table
{
	R(*invoke)(void* object, Ps... args) noexcept(N);
	void(*relocate)(void* src, void* dst);
	void(*destroy)(void* object);
};

template<typename T, bool N, typename R, typename... Ps>
inline constexpr _function_table<N, R, Ps...> _function_table_v =
{
	_function_invoke<T, N, R, Ps...>,

	std::is_trivially_copyable_v<remove_ref_t<T>>
		? _function_relocate_trivial<sizeof(remove_ref_t<T>)>
		: _function_relocate<remove_ref_t<T>>,

	std::is_trivially_destructible_v<remove_ref_t<T>>
		? _function_destroy_trivial
		: _function_destroy<remove_ref_t<T>>,
};

vsm_msvc_warning(pop)

template<size_t Capacity>
struct _function_storage
{
	alignas(std::max_align_t) unsigned char storage[Capacity];

	constexpr _function_storage()
	{
		vsm_if_consteval
		{
			for (size_t i = 0; i < Capacity; ++i)
			{
				storage[i] = 0;
			}
		}
	}
};

template<size_t Capacity, bool N, typename R, typename... Ps>
struct _function_base
{
	using table_type = _function_table<N, R, Ps...>;

	table_type const* m_table = nullptr;
	_function_storage<Capacity> m_storage;

	_function_base() = default;

	_function_base(_function_base&& other)
		: m_table(other.m_table)
	{
		if (m_table != nullptr)
		{
			m_table->relocate(
				other.m_storage.storage,
				m_storage.storage);
		}
		other.m_table = nullptr;
	}

	_function_base& operator=(_function_base&& other) &
	{
		if (m_table != nullptr)
		{
			m_table->destroy(m_storage.storage);
			m_table = nullptr;
		}

		if (other.m_table != nullptr)
		{
			m_table->relocate(
				other.m_storage.storage,
				m_storage.storage);

			m_table = other.m_table;
			other.m_table = nullptr;
		}
	}

	~_function_base()
	{
		if (m_table != nullptr)
		{
			m_table->destroy(m_storage.storage);
		}
	}


	template<typename T, typename Q, typename... Args>
	void construct(Args&&... args)
	{
		m_table = &_function_table_v<Q, N, R, Ps...>;
		::new (m_storage.storage) T(vsm_forward(args)...);
	}

	[[nodiscard]] explicit operator bool() const
	{
		return m_storage != nullptr;
	}

	template<std::convertible_to<Ps>... Args>
	R invoke(Args&&... args) const noexcept(N)
	{
		vsm_assert(m_table != nullptr);
		return m_table->invoke(
			const_cast<void*>(static_cast<void const*>(m_storage.storage)),
			vsm_forward(args)...);
	}
};

template<size_t Capacity, typename Signature>
struct _function_call;

#if 1 // auto-generated

template<size_t Capacity, typename R, typename... Ps>
struct _function_call<Capacity, R(Ps...)>
	: _function_base<Capacity, false, R, Ps...>
{
	using result_type = R;

	template<typename U>
	using view_type = U&;

	template<typename U>
	static constexpr bool is_invocable =
		std::is_invocable_r_v<R, U, Ps...> &&
		std::is_invocable_r_v<R, U&, Ps...>;

	template<std::convertible_to<Ps>... Args>
	R operator()(Args&&... args)
	{
		return this->invoke(vsm_forward(args)...);
	}
};


template<size_t Capacity, typename R, typename... Ps>
struct _function_call<Capacity, R(Ps...) &>
	: _function_base<Capacity, false, R, Ps...>
{
	using result_type = R;

	template<typename U>
	using view_type = U&;

	template<typename U>
	static constexpr bool is_invocable =
		std::is_invocable_r_v<R, U&, Ps...>;

	template<std::convertible_to<Ps>... Args>
	R operator()(Args&&... args) &
	{
		return this->invoke(vsm_forward(args)...);
	}
};


template<size_t Capacity, typename R, typename... Ps>
struct _function_call<Capacity, R(Ps...) &&>
	: _function_base<Capacity, false, R, Ps...>
{
	using result_type = R;

	template<typename U>
	using view_type = U&&;

	template<typename U>
	static constexpr bool is_invocable =
		std::is_invocable_r_v<R, U, Ps...>;

	template<std::convertible_to<Ps>... Args>
	R operator()(Args&&... args) &&
	{
		return this->invoke(vsm_forward(args)...);
	}
};


template<size_t Capacity, typename R, typename... Ps>
struct _function_call<Capacity, R(Ps...) const>
	: _function_base<Capacity, false, R, Ps...>
{
	using result_type = R;

	template<typename U>
	using view_type = U const&;

	template<typename U>
	static constexpr bool is_invocable =
		std::is_invocable_r_v<R, U const, Ps...> &&
		std::is_invocable_r_v<R, U const&, Ps...>;

	template<std::convertible_to<Ps>... Args>
	R operator()(Args&&... args) const
	{
		return this->invoke(vsm_forward(args)...);
	}
};


template<size_t Capacity, typename R, typename... Ps>
struct _function_call<Capacity, R(Ps...) const&>
	: _function_base<Capacity, false, R, Ps...>
{
	using result_type = R;

	template<typename U>
	using view_type = U const&;

	template<typename U>
	static constexpr bool is_invocable =
		std::is_invocable_r_v<R, U const&, Ps...>;

	template<std::convertible_to<Ps>... Args>
	R operator()(Args&&... args) const&
	{
		return this->invoke(vsm_forward(args)...);
	}
};


template<size_t Capacity, typename R, typename... Ps>
struct _function_call<Capacity, R(Ps...) const&&>
	: _function_base<Capacity, false, R, Ps...>
{
	using result_type = R;

	template<typename U>
	using view_type = U const&&;

	template<typename U>
	static constexpr bool is_invocable =
		std::is_invocable_r_v<R, U const, Ps...>;

	template<std::convertible_to<Ps>... Args>
	R operator()(Args&&... args) const&&
	{
		return this->invoke(vsm_forward(args)...);
	}
};


template<size_t Capacity, typename R, typename... Ps>
struct _function_call<Capacity, R(Ps...) noexcept>
	: _function_base<Capacity, true, R, Ps...>
{
	using result_type = R;

	template<typename U>
	using view_type = U&;

	template<typename U>
	static constexpr bool is_invocable =
		std::is_nothrow_invocable_r_v<R, U, Ps...> &&
		std::is_nothrow_invocable_r_v<R, U&, Ps...>;

	template<std::convertible_to<Ps>... Args>
	R operator()(Args&&... args) noexcept
	{
		return this->invoke(vsm_forward(args)...);
	}
};


template<size_t Capacity, typename R, typename... Ps>
struct _function_call<Capacity, R(Ps...) & noexcept>
	: _function_base<Capacity, true, R, Ps...>
{
	using result_type = R;

	template<typename U>
	using view_type = U&;

	template<typename U>
	static constexpr bool is_invocable =
		std::is_nothrow_invocable_r_v<R, U&, Ps...>;

	template<std::convertible_to<Ps>... Args>
	R operator()(Args&&... args) & noexcept
	{
		return this->invoke(vsm_forward(args)...);
	}
};


template<size_t Capacity, typename R, typename... Ps>
struct _function_call<Capacity, R(Ps...) && noexcept>
	: _function_base<Capacity, true, R, Ps...>
{
	using result_type = R;

	template<typename U>
	using view_type = U&&;

	template<typename U>
	static constexpr bool is_invocable =
		std::is_nothrow_invocable_r_v<R, U, Ps...>;

	template<std::convertible_to<Ps>... Args>
	R operator()(Args&&... args) && noexcept
	{
		return this->invoke(vsm_forward(args)...);
	}
};


template<size_t Capacity, typename R, typename... Ps>
struct _function_call<Capacity, R(Ps...) const noexcept>
	: _function_base<Capacity, true, R, Ps...>
{
	using result_type = R;

	template<typename U>
	using view_type = U const&;

	template<typename U>
	static constexpr bool is_invocable =
		std::is_nothrow_invocable_r_v<R, U const, Ps...> &&
		std::is_nothrow_invocable_r_v<R, U const&, Ps...>;

	template<std::convertible_to<Ps>... Args>
	R operator()(Args&&... args) const noexcept
	{
		return this->invoke(vsm_forward(args)...);
	}
};


template<size_t Capacity, typename R, typename... Ps>
struct _function_call<Capacity, R(Ps...) const& noexcept>
	: _function_base<Capacity, true, R, Ps...>
{
	using result_type = R;

	template<typename U>
	using view_type = U const&;

	template<typename U>
	static constexpr bool is_invocable =
		std::is_nothrow_invocable_r_v<R, U const&, Ps...>;

	template<std::convertible_to<Ps>... Args>
	R operator()(Args&&... args) const& noexcept
	{
		return this->invoke(vsm_forward(args)...);
	}
};


template<size_t Capacity, typename R, typename... Ps>
struct _function_call<Capacity, R(Ps...) const&& noexcept>
	: _function_base<Capacity, true, R, Ps...>
{
	using result_type = R;

	template<typename U>
	using view_type = U const&&;

	template<typename U>
	static constexpr bool is_invocable =
		std::is_nothrow_invocable_r_v<R, U const, Ps...>;

	template<std::convertible_to<Ps>... Args>
	R operator()(Args&&... args) const&& noexcept
	{
		return this->invoke(vsm_forward(args)...);
	}
};

#endif // auto-generated

inline constexpr size_t function_default_capacity = 2 * sizeof(void*);

} // namespace vsm::detail
