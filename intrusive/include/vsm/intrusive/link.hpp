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

struct links;

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

template<typename T, size_t MinSize, typename Tag = void>
concept linkable = requires (T const& object)
{
	detail::match_link<Tag, MinSize>(object);
};

template<typename T, typename Tag, typename Hook>
	requires linkable<T, size_for<Hook>, Tag>
inline constexpr bool check = true;

template<typename T, typename Tag, size_t MinSize = 0>
using link_t = decltype(detail::match_link<Tag, MinSize>(std::declval<T const&>()));

template<size_t Size, typename Tag>
constexpr size_t _link_size(basic_link<Size, Tag>*)
{
	return Size;
}

template<typename Link>
inline constexpr size_t link_size = detail::_link_size(static_cast<Link*>(0));

struct links
{
	template<typename Hook, typename Tag, size_t Size>
	vsm_always_inline static Hook* construct(basic_link<Size, std::type_identity_t<Tag>>* link)
	{
		static_assert(sizeof(link->m_storage) >= sizeof(Hook));
		static_assert(alignof(void*) >= alignof(Hook));
		return ::new (&link->m_storage) Hook;
	}

	template<typename Hook, typename Tag, size_t Size>
	vsm_always_inline static Hook* get_hook(basic_link<Size, std::type_identity_t<Tag>>* const link)
	{
		return std::launder(reinterpret_cast<Hook*>(&link->m_storage));
	}

	template<typename Hook, typename Tag, size_t Size>
	vsm_always_inline static Hook const* get_hook(basic_link<Size, std::type_identity_t<Tag>> const* const link)
	{
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
		using storage_type = unsigned char[link_size<link_type>];

		return static_cast<T const*>(reinterpret_cast<link_type const*>(
			std::launder(reinterpret_cast<storage_type const*>(hook))));
	}
};

} // namespace detail

template<size_t Size, typename Tag>
class basic_link
{
	alignas(void*) unsigned char m_storage[Size * sizeof(void*)];

	friend struct detail::links;
};

template<typename Tag>
class basic_link<0, Tag>
{
};

} // namespace vsm::intrusive
