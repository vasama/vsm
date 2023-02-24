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
namespace detail {

#define vsm_detail_heap_hook_from_children(children) \
	reinterpret_cast<hook*>(children)


struct _heap
{
	struct hook
	{
		hook* children[2];

		// Pointer to hook::children[0] of a parent hook or _heap::m_root.
		hook** parent;
	};

	typedef bool comparator(_heap const& self, hook const* lhs, hook const* rhs);


	hook* m_root = {};
	size_t m_size = {};


	_heap() = default;

	_heap(_heap&& other) noexcept
		: m_root(other.m_root)
		, m_size(other.m_size)
	{
		other.m_root = {};
		other.m_size = {};
	}

	_heap& operator=(_heap&& other) & noexcept
	{
		if (m_root != nullptr)
		{
			clear();
		}

		m_root = other.m_root;
		m_size = other.m_size;
		other.m_root = {};
		other.m_size = {};

		return *this;
	}

	~_heap()
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

} // namespace detail

template<typename Tag>
using basic_heap_link = basic_link<3, Tag>;

using heap_link = basic_heap_link<void>;

template<
	typename T,
	typename Comparator,
	typename KeySelector = identity_key_selector>
class heap : detail::_heap
{
public:
	using element_type = detail::element_t<T>;
	using tag_type = detail::tag_t<T>;

	using key_type = decltype(std::declval<KeySelector const&>()(std::declval<T const&>()));

private:
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
	[[nodiscard]] element_type* peek()
	{
		vsm_assert(m_size > 0);
		return get_elem(m_root);
	}

	/// @return The minimum element of the heap.
	/// @pre The heap is not empty.
	[[nodiscard]] element_type const* peek() const
	{
		vsm_assert(m_size > 0);
		return get_elem(m_root);
	}


	/// @brief Insert an element into the heap.
	/// @param element Element to be inserted.
	/// @pre @p element is not part of any container.
	void push(element_type* const element)
	{
		_heap::push(detail::links::construct<hook, tag_type>(element), comparator);
	}

	/// @brief Remove an element from the heap.
	/// @param element Element to be removed.
	/// @pre @p element is part of this heap.
	void remove(element_type* const element)
	{
		_heap::remove(get_hook(element), comparator);
	}

	/// @brief Pop the minimum element of the heap.
	/// @return The minimum element.
	/// @pre The heap is not empty.
	[[nodiscard]] element_type* pop()
	{
		return get_elem(_heap::pop(comparator));
	}


	friend void swap(heap& lhs, heap& rhs) noexcept
	{
		using std::swap;
		swap(static_cast<_heap&>(lhs), static_cast<_heap&>(rhs));
		swap(lhs.m_key_selector, rhs.m_key_selector);
		swap(lhs.m_comparator, rhs.m_comparator);
	}

private:
	[[nodiscard]] static auto* get_hook(auto* const element)
	{
		return detail::links::get_hook<hook, tag_type>(element);
	}

	[[nodiscard]] static auto* get_elem(auto* const hook)
	{
		return detail::links::get_elem<element_type, tag_type>(hook);
	}

	static bool comparator(_heap const& base, hook const* const lhs, hook const* const rhs)
	{
		heap const& self = static_cast<heap const&>(base);
		return self.m_comparator(
			self.m_key_selector(*get_elem(lhs)),
			self.m_key_selector(*get_elem(rhs)));
	}
};

template<typename T, typename KeySelector = identity_key_selector>
using max_heap = heap<T, std::less<>, KeySelector>;

template<typename T, typename KeySelector = identity_key_selector>
using min_heap = heap<T, std::greater<>, KeySelector>;

} // namespace vsm::intrusive
