#pragma once

#include <vsm/platform.h>

#include <new>
#include <type_traits>
#include <utility>

#include <cstddef>
#include <cstdint>

namespace vsm::intrusive {

template<typename T, typename Tag>
struct tag;

template<size_t Size, typename Tag = void>
class basic_link;

namespace detail {

template<typename T>
inline constexpr size_t size_for = (sizeof(T) + sizeof(void*) - 1) / sizeof(void*);

template<typename T>
struct _element
{
	using type = T;
	using tag_type = void;
};

template<typename T, typename Tag>
struct _element<tag<T, Tag>>
{
	using type = T;
	using tag_type = Tag;
};

template<typename T>
using element_t = typename _element<T>::type;

template<typename T>
using tag_t = typename _element<T>::tag_type;

template<typename Tag, size_t MinSize, size_t Size>
	requires (Size >= MinSize)
basic_link<Size, Tag> match_link(basic_link<Size, Tag> const&);

class linker;

class access
{
	template<typename T, typename Tag, size_t MinSize = 0>
	using link_t = decltype(detail::match_link<Tag, MinSize>(std::declval<T const&>()));

	template<typename Hook, typename Tag, typename T>
	vsm_always_inline static Hook* construct(T* const object)
	{
		link_t<T, Tag>* const link = object;
		static_assert(sizeof(link->m_storage) >= sizeof(Hook));
		static_assert(alignof(void*) >= alignof(Hook));
		return ::new (&link->m_storage) Hook;
	}

	template<typename Hook, typename Tag, typename T>
	vsm_always_inline static Hook* get_hook(T* const object)
	{
		link_t<T, Tag>* const link = object;
		return std::launder(reinterpret_cast<Hook*>(&link->m_storage));
	}

	template<typename Hook, typename Tag, typename T>
	vsm_always_inline static Hook const* get_hook(T const* const object)
	{
		link_t<T, Tag> const* const link = object;
		return std::launder(reinterpret_cast<Hook const*>(&link->m_storage));
	}

	template<typename T, typename Tag, typename Hook>
	vsm_always_inline static T* get_elem(Hook* const hook)
	{
		using link_type = link_t<T, Tag, size_for<Hook>>;
		using storage_type = decltype(link_type::m_storage);

		return static_cast<T*>(reinterpret_cast<link_type*>(
			std::launder(reinterpret_cast<storage_type*>(hook))));
	}

	template<typename T, typename Tag, typename Hook>
	vsm_always_inline static T const* get_elem(Hook const* const hook)
	{
		using link_type = link_t<T, Tag>;
		using storage_type = decltype(link_type::m_storage);

		return static_cast<T const*>(reinterpret_cast<link_type const*>(
			std::launder(reinterpret_cast<storage_type const*>(hook))));
	}

	friend linker;
};

class linker : access
{
public:
	template<typename T, typename Tag, size_t MinSize = 0>
	using link_t = access::link_t<T, Tag, MinSize>;

	using access::construct;
	using access::get_hook;
	using access::get_elem;
};

template<typename T, size_t MinSize, typename Tag = void>
concept linkable = requires (T const& object)
{
	typename linker::link_t<T, Tag, MinSize>;
};

template<typename T, typename Tag, typename Hook>
	requires linkable<T, size_for<Hook>, Tag>
inline constexpr bool check = true;

} // namespace detail

using detail::access;

template<size_t Size, typename Tag>
class basic_link
{
	alignas(void*) unsigned char m_storage[Size * sizeof(void*)];

	friend detail::access;
};

template<typename Tag>
class basic_link<0, Tag>
{
};

} // namespace vsm::intrusive
