#pragma once

#include <vsm/intrusive/link.hpp>

#include <vsm/assert.h>
#include <vsm/key_selector.hpp>
#include <vsm/linear.hpp>
#include <vsm/utility.hpp>

#include <concepts>
#include <type_traits>
#include <utility>

namespace vsm::intrusive {

using heap_link = link<3>;

namespace detail::heap_ {

#define vsm_heap_hook(element, ...) \
	(reinterpret_cast<heap_::hook __VA_ARGS__*>(static_cast<heap_link __VA_ARGS__*>(element)))

#define vsm_heap_elem(hook, ...) \
	(static_cast<T __VA_ARGS__*>(reinterpret_cast<heap_link __VA_ARGS__*>(hook)))


struct hook : link_base
{
	hook* children[2];

	// Pointer to hook::children[0] of a parent hook or base::m_root.
	hook** parent;
};

struct base : link_container
{
	typedef bool comparator(base const& self, hook const* lhs, hook const* rhs);


	linear<hook*> m_root;
	linear<size_t> m_size;


	base(base&&) = default;

	base& operator=(base&& src) & noexcept
	{
		if (m_root.value != nullptr)
		{
			clear();
		}
		m_root = vsm_move(src.m_root);
		m_size = vsm_move(src.m_size);
		return *this;
	}

	~base()
	{
		if (m_root.value != nullptr)
		{
			clear();
		}
	}

	void push(hook* node, comparator* cmp);
	void remove(hook* node, comparator* cmp);
	hook* pop(comparator* cmp);
	void clear();
};


template<std::derived_from<heap_link> T,
	typename Comparator,
	key_selector<T> KeySelector = identity_key_selector>
class heap : base
{
	using key_type = decltype(std::declval<KeySelector const&>()(std::declval<T const&>()));

	static constexpr bool noexcept_compare =
		noexcept(std::declval<Comparator const&>()(
			std::declval<key_type const&>(), std::declval<key_type const&>()));

	[[no_unique_address]] KeySelector m_key_selector;
	[[no_unique_address]] Comparator m_comparator;

public:
	heap() = default;

	explicit constexpr heap(KeySelector key_selector) noexcept
		: m_key_selector(vsm_move(key_selector))
	{
	}

	explicit constexpr heap(Comparator comparator) noexcept
		: m_comparator(vsm_move(comparator))
	{
	}

	explicit constexpr heap(KeySelector key_selector, Comparator comparator) noexcept
		: m_key_selector(vsm_move(key_selector))
		, m_comparator(vsm_move(comparator))
	{
	}

	heap& operator=(heap&&) & = default;


	/// @return Size of the heap.
	[[nodiscard]] size_t size() const noexcept
	{
		return m_size.value;
	}

	/// @return True if the heap is empty.
	[[nodiscard]] bool empty() const noexcept
	{
		return m_size.value == 0;
	}


	/// @return The minimum element of the heap.
	/// @pre The heap is not empty.
	[[nodiscard]] T* peek() noexcept
	{
		vsm_assert(m_size.value > 0);
		return vsm_heap_elem(m_root.value);
	}

	/// @return The minimum element of the heap.
	/// @pre The heap is not empty.
	[[nodiscard]] T const* peek() const noexcept
	{
		vsm_assert(m_size.value > 0);
		return vsm_heap_elem(m_root.value);
	}


	/// @brief Insert an element into the heap.
	/// @param element Element to be inserted.
	/// @pre @p element is not part of any container.
	void push(T* const element) noexcept(noexcept_compare)
	{
		base::push(vsm_heap_hook(element), comparator);
	}

	/// @brief Remove an element from the heap.
	/// @param element Element to be removed.
	/// @pre @p element is part of this heap.
	void remove(T* const element) noexcept(noexcept_compare)
	{
		base::remove(vsm_heap_hook(element), comparator);
	}

	/// @brief Pop the minimum element of the heap.
	/// @return The minimum element.
	/// @pre The heap is not empty.
	[[nodiscard]] T* pop() noexcept(noexcept_compare)
	{
		return vsm_heap_elem(base::pop(comparator));
	}


	friend void swap(heap& lhs, heap& rhs) noexcept
	{
		using std::swap;
		swap(static_cast<base&>(lhs), static_cast<base&>(rhs));
		swap(lhs.m_key_selector, rhs.m_key_selector);
		swap(lhs.m_comparator, rhs.m_comparator);
	}

private:
	static bool comparator(base const& self_base, hook const* const lhs, hook const* const rhs)
	{
		heap const& self = static_cast<heap const&>(self_base);
		return self.m_comparator(
			self.m_key_selector(*vsm_heap_elem(lhs, const)),
			self.m_key_selector(*vsm_heap_elem(rhs, const)));
	}
};

#undef vsm_heap_hook
#undef vsm_heap_elem

} // namespace detail::heap_

using detail::heap_::heap;

template<std::derived_from<heap_link> T, key_selector<T> KeySelector = identity_key_selector>
using max_heap = heap<T, KeySelector, std::less<>>;

template<std::derived_from<heap_link> T, key_selector<T> KeySelector = identity_key_selector>
using min_heap = heap<T, KeySelector, std::greater<>>;

} // namespace vsm::intrusive
