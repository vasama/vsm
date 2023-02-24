#pragma once

#include <concepts>

#include <cstddef>

namespace vsm {

struct allocation
{
	void* buffer;
	size_t size;

	operator void*() const
	{
		return buffer;
	}
};

template<typename T>
concept allocator = requires (T& t, size_t const& s, allocation const& a)
{
	// allocation allocate(size_t min_size);
	{ t.allocate(s) } -> std::same_as<allocation>;
	
	// void deallocate(allocation allocation);
	{ t.deallocate(a) } -> std::same_as<void>;
};


namespace allocators {
namespace detail {

template<typename T>
consteval bool detect_is_always_equal()
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
consteval bool detect_is_propagatable()
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

template<allocator T>
inline constexpr bool is_always_equal_v = detail::detect_is_always_equal<T>();

template<allocator T>
inline constexpr bool is_propagatable_v = detail::detect_is_propagatable<T>();

template<allocator T>
inline constexpr bool has_resize_v = requires (T& t, size_t const& s, allocation const& a)
{
	// size_t resize(allocation allocation, size_t min_size);
	{ t.resize(a, s) } -> std::same_as<size_t>;
};

template<allocator T>
size_t resize(T& allocator, allocation const allocation, size_t const min_size)
{
	if constexpr (has_resize_v<T>)
	{
		return allocator.resize(allocation, min_size);
	}
	else
	{
		return 0;
	}
}

} //namespace allocators
} // namespace vsm
