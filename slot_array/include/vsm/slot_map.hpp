#pragma once

#include <concepts>
#include <memory>

namespace vsm {
namespace detail {

template<typename K, typename V, typename Allocator = std::allocator<T>>
class slot_map : Allocator
{
	T* m_data;
	size_t* m_keys;
	size_t* m_ridx;
	size_t m_size;

public:
	template<std::convertible_to<T> U = T>
	K insert(U&& value)
	{

	}

	T* try_get(K const key)
	{

	}

	T const* try_get(K const key) const
	{

	}
};

} // namespace detail



} // namespace vsm
