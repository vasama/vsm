#pragma once

#include <vsm/intrusive/link.hpp>

#include <vsm/assert.h>
#include <vsm/key_selector.hpp>
#include <vsm/standard.hpp>
#include <vsm/utility.hpp>

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

namespace vsm::intrusive {

using heap_link = link<3>;

namespace detail::heap_ {

#define vsm_detail_heap_hook(element, ...) \
	(reinterpret_cast<heap_::hook __VA_ARGS__*>(static_cast<heap_link __VA_ARGS__*>(element)))

#define vsm_detail_heap_elem(hook, ...) \
	(static_cast<T __VA_ARGS__*>(reinterpret_cast<heap_link __VA_ARGS__*>(hook)))

#define vsm_detail_heap_hook_from_children(children) \
	static_cast<hook*>(reinterpret_cast<hook_data*>(children))


struct hook;

struct hook_data
{
	hook* children[2];

	// Pointer to hook::children[0] of a parent hook or base::m_root.
	hook** parent;
};

struct hook : link_base, hook_data {};

struct base;

typedef bool comparator(base const& self, hook const* lhs, hook const* rhs);

struct base : link_container
{
	hook* m_root = {};
	size_t m_size = {};


	base() = default;

	base(base&& other) noexcept
		: link_container(static_cast<link_container&&>(other))
		, m_root(other.m_root)
		, m_size(other.m_size)
	{
		other.m_root = {};
		other.m_size = {};
	}

	base& operator=(base&& other) & noexcept
	{
		if (m_root != nullptr)
		{
			clear();
		}

		static_cast<link_container&>(*this) = static_cast<link_container&&>(other);

		m_root = other.m_root;
		m_size = other.m_size;
		other.m_root = {};
		other.m_size = {};

		return *this;
	}

	~base()
	{
		if (m_root != nullptr)
		{
			clear();
		}
	}

	void push(hook* node, comparator* cmp);
	void remove(hook* node, comparator* cmp);
	hook* pop(comparator* cmp);
	void clear();
};


template<
	typename T, typename Comparator,
	typename KeySelector = identity_key_selector>
class heap : base
{
	static_assert(std::derived_from<T, heap_link>);
	static_assert(key_selector<KeySelector, T>);

	using key_type = decltype(std::declval<KeySelector const&>()(std::declval<T const&>()));

	vsm_no_unique_address KeySelector m_key_selector;
	vsm_no_unique_address Comparator m_comparator;

public:
	heap() = default;

	explicit constexpr heap(KeySelector key_selector)
		: m_key_selector(vsm_move(key_selector))
	{
	}

	explicit constexpr heap(Comparator comparator)
		: m_comparator(vsm_move(comparator))
	{
	}

	explicit constexpr heap(KeySelector key_selector, Comparator comparator)
		: m_key_selector(vsm_move(key_selector))
		, m_comparator(vsm_move(comparator))
	{
	}

	heap& operator=(heap&&) & = default;


	/// @return Size of the heap.
	[[nodiscard]] size_t size() const
	{
		return m_size;
	}

	/// @return True if the heap is empty.
	[[nodiscard]] bool empty() const
	{
		return m_size == 0;
	}


	/// @return The minimum element of the heap.
	/// @pre The heap is not empty.
	[[nodiscard]] T* peek()
	{
		vsm_assert(m_size > 0);
		return vsm_detail_heap_elem(m_root);
	}

	/// @return The minimum element of the heap.
	/// @pre The heap is not empty.
	[[nodiscard]] T const* peek() const
	{
		vsm_assert(m_size > 0);
		return vsm_detail_heap_elem(m_root);
	}


	/// @brief Insert an element into the heap.
	/// @param element Element to be inserted.
	/// @pre @p element is not part of any container.
	void push(T* const element)
	{
		base::push(vsm_detail_heap_hook(element), comparator);
	}

	/// @brief Remove an element from the heap.
	/// @param element Element to be removed.
	/// @pre @p element is part of this heap.
	void remove(T* const element)
	{
		base::remove(vsm_detail_heap_hook(element), comparator);
	}

	/// @brief Pop the minimum element of the heap.
	/// @return The minimum element.
	/// @pre The heap is not empty.
	[[nodiscard]] T* pop()
	{
		return vsm_detail_heap_elem(base::pop(comparator));
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
			self.m_key_selector(*vsm_detail_heap_elem(lhs, const)),
			self.m_key_selector(*vsm_detail_heap_elem(rhs, const)));
	}
};

#undef vsm_detail_heap_hook
#undef vsm_detail_heap_elem

} // namespace detail::heap_

using detail::heap_::heap;

template<typename T, typename KeySelector = identity_key_selector>
using max_heap = heap<T, std::less<>, KeySelector>;

template<typename T, typename KeySelector = identity_key_selector>
using min_heap = heap<T, std::greater<>, KeySelector>;

} // namespace vsm::intrusive
