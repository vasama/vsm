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

	[[nodiscard]] constexpr operator void*() const noexcept
	{
		return buffer;
	}
};

namespace detail {

template<typename T>
inline constexpr bool _allocator_has_resize = requires (T& t, size_t const& s, allocation const& a)
{
	// size_t resize(allocation allocation, size_t min_size) /* const */;
	{ t.resize(a, s) } noexcept -> std::same_as<size_t>;
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

template<typename T, memory_resource Allocator>
constexpr void delete_via(T* const object, Allocator&& allocator)
{
	if (object != nullptr)
	{
		object->~T();
		allocator.deallocate(allocation(object, sizeof(T)));
	}
}


class new_allocator
{
public:
	static constexpr bool is_always_equal = true;
	static constexpr bool is_propagatable = true;

	[[nodiscard]] allocation allocate(size_t const size) const noexcept
	{
		return { operator new(size, std::nothrow), size };
	}

	void deallocate(allocation const allocation) const noexcept
	{
		operator delete(allocation.buffer, allocation.size);
	}
};

using default_allocator = new_allocator;

} // namespace vsm

template<vsm::memory_resource Allocator>
[[nodiscard]] constexpr void* operator new(size_t const size, Allocator&& allocator)
{
	return vsm::allocate_or_throw(allocator, size);
}

template<vsm::memory_resource Allocator>
[[nodiscard]] constexpr void* operator new(
	size_t const size,
	Allocator&& allocator,
	std::nothrow_t) noexcept
{
	return allocator.allocate(size);
}
