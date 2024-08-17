#pragma once

#include <vsm/exceptions.hpp>
#include <vsm/type_traits.hpp>

#include <concepts>
#include <new>

#include <cstddef>

namespace vsm {

struct allocation
{
	void* buffer;
	size_t size;

	[[nodiscard]] constexpr operator void*() const
	{
		return buffer;
	}
};

namespace detail {

template<typename T>
inline constexpr bool _allocator_has_resize = requires (T& t, size_t const& s, allocation const& a)
{
	// size_t resize(allocation allocation, size_t min_size) /* const */;
	{ t.resize(a, s) } -> std::same_as<size_t>;
};

template<typename T>
consteval bool _allocator_is_always_equal()
{
	if constexpr (requires { T::is_always_equal; })
	{
		return T::is_always_equal;
	}
	else
	{
		return std::is_empty_v<T>;
	}
}

template<typename T>
consteval bool _allocator_is_propagatable()
{
	if constexpr (requires { T::is_propagatable; })
	{
		return T::is_propagatable;
	}
	else
	{
		return false;
	}
}

} // namespace detail

template<typename T>
concept memory_resource = requires (T& t, size_t const& s, allocation const& a)
{
	// allocation allocate(size_t min_size) /* const */;
	{ t.allocate(s) } -> std::same_as<allocation>;
	
	// void deallocate(allocation allocation) /* const */;
	{ t.deallocate(a) } -> std::same_as<void>;
};

template<typename T>
concept allocator =
	memory_resource<remove_cvref_t<T> const> &&
	std::is_copy_constructible_v<T> &&
	std::is_nothrow_move_constructible_v<T>;

namespace allocators {

template<allocator T>
inline constexpr bool is_always_equal_v = detail::_allocator_is_always_equal<T>();

template<allocator T>
inline constexpr bool is_propagatable_v = detail::_allocator_is_propagatable<T>();

template<memory_resource T>
inline constexpr bool has_resize_v = detail::_allocator_has_resize<T const>;

template<memory_resource Allocator>
[[nodiscard]] constexpr size_t resize(
	Allocator&& allocator,
	allocation const allocation,
	size_t const min_size)
{
	if constexpr (has_resize_v<Allocator>)
	{
		return allocator.resize(allocation, min_size);
	}
	else
	{
		return 0;
	}
}

} // namespace allocators

template<memory_resource Allocator>
[[nodiscard]] constexpr allocation allocate_or_throw(
	Allocator&& allocator,
	size_t const min_size)
{
	auto const allocation = allocator.allocate(min_size);
	if (allocation.buffer == nullptr)
	{
		vsm_except_throw_or_terminate(std::bad_alloc());
	}
	return allocation;
}

template<memory_resource Allocator, typename T>
constexpr void delete_via(Allocator&& allocator, T* const object)
{
	if (object != nullptr)
	{
		object->~T();
		allocator.deallocate(allocation(object, sizeof(object)));
	}
}

#if 0
template<typename... Args>
class delete_with
{
	std::tuple<Args&&...> m_args;

public:
	explicit delete_with(Args&&... args)
		: m_args(vsm_forward(args)...)
	{
	}
	
	delete_with(delete_with const&) = delete;
	delete_with& operator=(delete_with const&) = delete;

	template<typename Self, typename T>
	void operator()(this Self&& self, T const* const object) noexcept
	{
		if (object != nullptr)
		{
			object->~T();
			std::apply(
				[](auto&&... args)
				{
					auto const ptr = const_cast<void*>(static_cast<void const*>(object));

					if constexpr (requires { operator delete(ptr, sizeof(T), vsm_forward(args)...); })
					{
						operator delete(ptr, sizeof(T), vsm_forward(args)...);
					}
					else
					{
						operator delete(ptr, vsm_forward(args)...);
					}
				},
				vsm_forward(self).m_args);
		}
	}
};
#endif

} // namespace vsm

template<vsm::memory_resource Allocator>
[[nodiscard]] constexpr void* operator new(
	size_t const size,
	Allocator&& allocator)
{
	return vsm::allocate_or_throw(allocator, size);
}

template<vsm::memory_resource Allocator>
[[nodiscard]] constexpr void* operator new(
	size_t const size,
	Allocator&& allocator,
	std::nothrow_t)
{
	return allocator.allocate(size);
}

#if 0
template<vsm::memory_resource Allocator>
constexpr void operator delete(
	void* const block,
	size_t const size,
	Allocator&& allocator)
{
	//TODO: Test that this is called correctly on exception thrown from constructor.
	return allocator.deallocate(vsm::allocation(block, size));
}

template<vsm::memory_resource Allocator>
constexpr void operator delete(
	void* const block,
	size_t const size,
	Allocator&& allocator,
	std::nothrow_t)
{
	//TODO: Test that this is called correctly on exception thrown from constructor.
	return allocator.deallocate(vsm::allocation(block, size));
}
#endif
