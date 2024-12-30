#pragma once

#include <vsm/assert.h>
#include <vsm/concepts.hpp>
#include <vsm/exceptions.hpp>
#include <vsm/type_traits.hpp>

#include <concepts>
#include <new>

#include <cstddef>

namespace vsm {

struct allocation
{
	void* storage;
	size_t size;

	explicit allocation(decltype(nullptr))
		: storage(nullptr)
		, size(0)
	{
	}

	explicit allocation(void* const storage, size_t const size) noexcept
		: storage(storage)
		, size(size)
	{
	}

	template<any_cv_of<void> T>
	[[nodiscard]] operator T*() const noexcept
	{
		return storage;
	}

	template<no_cv_of<void> T>
		requires std::is_object_v<T>
	[[nodiscard]] explicit operator T*() const noexcept
	{
		return static_cast<T*>(storage);
	}
};

namespace detail {

template<typename T>
inline constexpr bool _allocator_has_resize = requires (T& t, size_t const& s, allocation const& a)
{
	// size_t resize(allocation allocation, size_t min_size, size_t max_size) /* const */;
	{ t.resize(a, s, s) } noexcept -> std::same_as<size_t>;
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

template<typename T>
typename T::position_type _allocator_position_type(int);

template<typename T>
void _allocator_position_type(...);

} // namespace detail

template<typename T>
concept memory_resource = requires (T& t, size_t const& s, allocation const& a)
{
	// allocation allocate(size_t min_size, size_t max_size) /* const */;
	{ t.allocate(s, s) } noexcept -> std::same_as<allocation>;

	// void deallocate(allocation allocation) /* const */;
	{ t.deallocate(a) } noexcept -> std::same_as<void>;
};

template<typename T>
concept allocator =
	memory_resource<remove_cvref_t<T> const> &&
	std::is_copy_constructible_v<T> &&
	std::is_nothrow_move_constructible_v<T>;


template<typename T>
concept managed_memory_resource = memory_resource<T> && requires (T& t)
{
	// void deallocate_all() /* const */;
	{ t.deallocate_all() } noexcept -> std::same_as<void>;
};

template<typename T>
concept managed_allocator = allocator<T> && managed_memory_resource<remove_cvref_t<T const>>;


template<typename T>
concept monotonic_memory_resource = memory_resource<T> && requires (T& t)
{
	requires std::is_trivially_copyable_v<typename T::position_type>;

	{ static_cast<T const&>(t).get_position() } noexcept
		-> std::same_as<typename T::position_type>;

	t.reset_position(std::declval<typename T::position_type const&>());
};

template<typename T>
concept monotonic_allocator = allocator<T> && monotonic_memory_resource<remove_cvref_t<T const>>;


namespace allocators {

template<memory_resource T>
inline constexpr bool has_resize_v = detail::_allocator_has_resize<T const>;


template<allocator T>
inline constexpr bool is_always_equal_v = detail::_allocator_is_always_equal<T>();

template<allocator T>
inline constexpr bool is_propagatable_v = detail::_allocator_is_propagatable<T>();


template<memory_resource Allocator>
[[nodiscard]] constexpr size_t resize(
	Allocator&& allocator,
	allocation const allocation,
	size_t const min_size)
{
	if constexpr (has_resize_v<Allocator>)
	{
		return allocator.resize(allocation, min_size, min_size);
	}
	else
	{
		return 0;
	}
}

template<memory_resource Allocator>
[[nodiscard]] constexpr size_t resize(
	Allocator&& allocator,
	allocation const allocation,
	size_t const min_size,
	size_t const max_size)
{
	vsm_assert(min_size <= max_size);

	if constexpr (has_resize_v<Allocator>)
	{
		return allocator.resize(allocation, min_size, max_size);
	}
	else
	{
		return 0;
	}
}


template<typename MemoryResource>
using position_type_or_void = decltype(detail::_allocator_position_type<MemoryResource>(0));

} // namespace allocators

template<memory_resource Allocator>
[[nodiscard]] constexpr allocation allocate(
	Allocator&& allocator,
	size_t const min_size)
{
	return allocator.allocate(min_size, min_size);
}

template<memory_resource Allocator>
[[nodiscard]] constexpr allocation allocate(
	Allocator&& allocator,
	size_t const min_size,
	size_t const max_size)
{
	return allocator.allocate(min_size, max_size);
}

template<memory_resource Allocator>
[[nodiscard]] constexpr allocation allocate_at_least(
	Allocator&& allocator,
	size_t const min_size)
{
	return allocator.allocate(min_size, static_cast<size_t>(-1));
}


template<memory_resource Allocator>
[[nodiscard]] constexpr allocation allocate_or_throw(
	Allocator&& allocator,
	size_t const min_size)
{
	auto const allocation = allocator.allocate(min_size, min_size);

	if (allocation.storage == nullptr)
	{
		vsm_except_throw_or_terminate(std::bad_alloc());
	}

	return allocation;
}

template<memory_resource Allocator>
[[nodiscard]] constexpr allocation allocate_or_throw(
	Allocator&& allocator,
	size_t const min_size,
	size_t const max_size)
{
	vsm_assert(min_size <= max_size);

	auto const allocation = allocator.allocate(min_size, max_size);

	if (allocation.storage == nullptr)
	{
		vsm_except_throw_or_terminate(std::bad_alloc());
	}

	return allocation;
}

template<memory_resource Allocator>
[[nodiscard]] constexpr allocation allocate_at_least_or_throw(
	Allocator&& allocator,
	size_t const min_size)
{
	auto const allocation = allocator.allocate(min_size, static_cast<size_t>(-1));

	if (allocation.storage == nullptr)
	{
		vsm_except_throw_or_terminate(std::bad_alloc());
	}

	return allocation;
}


template<non_decaying T, memory_resource Allocator, typename... Args>
	requires std::constructible_from<T, Args...>
[[nodiscard]] constexpr T* new_via(Allocator&& allocator, Args&&... args)
{
	auto const allocation = allocator.allocate(sizeof(T), sizeof(T));

	if (allocation.storage == nullptr)
	{
		vsm_except_throw_or_terminate(std::bad_alloc());
	}

	if constexpr (std::is_nothrow_constructible_v<T, Args...>)
	{
		return ::new (allocation.storage) T(static_cast<Args&&>(args)...);
	}
	else
	{
		vsm_except_try
		{
			return ::new (allocation.storage) T(static_cast<Args&&>(args)...);
		}
		vsm_except_catch (...)
		{
			allocator.deallocate(allocation);
			vsm_except_rethrow;
		}
	}
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


template<non_ref MemoryResource>
	requires memory_resource<MemoryResource>
class basic_allocator
{
	MemoryResource* m_memory_resource;

public:
	using position_type = allocators::position_type_or_void<MemoryResource>;

	basic_allocator(MemoryResource& m_memory_resource) noexcept
		: m_memory_resource(std::addressof(m_memory_resource))
	{
	}

	[[nodiscard]] vsm::allocation allocate(
		size_t const min_size,
		size_t const max_size) const noexcept
	{
		return m_memory_resource->allocate(min_size, max_size);
	}

	void deallocate(vsm::allocation const allocation) const noexcept
	{
		m_memory_resource->deallocate(allocation);
	}

	void deallocate_all() const noexcept
		requires managed_memory_resource<MemoryResource>
	{
		m_memory_resource->deallocate_all();
	}

	[[nodiscard]] size_t resize(
		vsm::allocation const allocation,
		size_t const min_size,
		size_t const max_size) const noexcept
		requires allocators::has_resize_v<MemoryResource>
	{
		return m_memory_resource->resize(allocation, min_size, max_size);
	}

	template<typename MR = MemoryResource>
		requires monotonic_memory_resource<MemoryResource>
	[[nodiscard]] typename MR::position_type get_position() const noexcept
	{
		return m_memory_resource->get_position();
	}

	template<typename MR = MemoryResource>
		requires monotonic_memory_resource<MemoryResource>
	void reset_position(typename MR::position_type const pos) const noexcept
	{
		m_memory_resource->reset_position(pos);
	}
};


class new_allocator
{
public:
	static constexpr bool is_always_equal = true;
	static constexpr bool is_propagatable = true;

	[[nodiscard]] allocation allocate(
		size_t const min_size,
		[[maybe_unused]] size_t const max_size) const noexcept
	{
		return allocation(::operator new(min_size, std::nothrow), min_size);
	}

	void deallocate(allocation const allocation) const noexcept
	{
		::operator delete(allocation.storage, allocation.size);
	}
};

using default_allocator = new_allocator;

} // namespace vsm

template<vsm::memory_resource Allocator>
[[nodiscard]] constexpr void* operator new(size_t const size, Allocator&& allocator)
{
	return vsm::allocate_or_throw(allocator, size).storage;
}

template<vsm::memory_resource Allocator>
[[nodiscard]] constexpr void* operator new(
	size_t const size,
	Allocator&& allocator,
	std::nothrow_t) noexcept
{
	return allocator.allocate(size, size);
}
