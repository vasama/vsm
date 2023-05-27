#pragma once

#include <vsm/assert.h>
#include <vsm/intrusive/link.hpp>

namespace vsm::intrusive {

using forward_list_link = link<1>;

namespace detail::forward_list_ {

#define vsm_detail_forward_list_hook(element) \
	(reinterpret_cast<detail::forward_list_::hook*>(static_cast<forward_list_link*>(element)))

#define vsm_detail_forward_list_elem(hook) \
	(static_cast<T*>(reinterpret_cast<forward_list_link*>(hook)))


struct hook : link_base
{
	hook* next;

	constexpr void loop() noexcept
	{
		next = this;
	}
};

struct base : link_container
{
	hook m_root;
	hook* m_tail;


	constexpr base() noexcept
	{
		m_root.loop();
		m_tail = &m_root;
	}

	void push_front(hook* head, hook* tail) noexcept;
	void push_front(hook* node) noexcept;
	void splice_front(base& list) noexcept;
	void push_back(hook* head, hook* tail) noexcept;
	void push_back(hook* node) noexcept;
	void splice_back(base& list) noexcept;
	hook* pop_front() noexcept;
	void clear() noexcept;
};


template<std::derived_from<forward_list_link> T>
class iterator
{
	hook* m_node;

public:
	using difference_type = ptrdiff_t;
	using value_type = T;
	using pointer = T*;
	using reference = T;


	iterator() = default;

	explicit iterator(hook* const node)
		: m_node(node)
	{
	}


	[[nodiscard]] T& operator*() const noexcept
	{
		return *vsm_detail_forward_list_elem(m_node);
	}

	[[nodiscard]] T* operator->() const noexcept
	{
		return vsm_detail_forward_list_elem(m_node);
	}
	
	
	iterator& operator++() const noexcept
	{
		m_node = m_node->next;
		return *this;
	}

	[[nodiscard]] iterator operator++(int) & noexcept
	{
		iterator result = *this;
		m_node = m_node->next;
		return result;
	}


	[[nodiscard]] friend bool operator==(iterator const&, iterator const&) = default;
};

} // namespace detail::forward_list_

template<std::derived_from<forward_list_link> T>
class forward_list : detail::forward_list_::base
{
public:
	using       iterator = detail::forward_list_::iterator<      T>;
	using const_iterator = detail::forward_list_::iterator<const T>;


	using base::base;

	forward_list& operator=(forward_list&&) & = default;


	/// @return True if the list is empty.
	[[nodiscard]] bool empty() const noexcept
	{
		return m_root.next == &m_root;
	}


	/// @return First element in the list.
	/// @pre The list is not empty.
	[[nodiscard]] T* front() noexcept
	{
		vsm_assert(m_root.next != &m_root);
		return vsm_detail_forward_list_elem(m_root.next);
	}

	/// @return First element in the list.
	/// @pre The list is not empty.
	[[nodiscard]] T const* front() const noexcept
	{
		vsm_assert(m_root.next != &m_root);
		return vsm_detail_forward_list_elem(m_root.next);
	}


	/// @brief Insert an element at the front of the list.
	/// @param element Element to be inserted.
	/// @pre @p element is not part of any container.
	void push_front(T* const element) noexcept
	{
		base::push_front(vsm_detail_forward_list_hook(element));
	}

	void splice_front(forward_list<T>&& other) noexcept
	{
		base::splice_front(other);
	}


	void push_back(T* const element) noexcept
	{
		base::push_back(vsm_detail_forward_list_hook(element));
	}

	void splice_back(forward_list<T>&& other) noexcept
	{
		base::splice_back(other);
	}


	/// @brief Remove the first element from the list.
	/// @return The first element in the list.
	/// @pre The list is not empty.
	[[nodiscard]] T* pop_front() noexcept
	{
		vsm_assert(m_root.next != &m_root);
		return vsm_detail_forward_list_elem(base::pop_front());
	}


	void clear() noexcept
	{
		if (m_root.next != &m_root)
		{
			base::clear();
		}
	}


	[[nodiscard]] iterator before_begin() noexcept
	{
		return iterator(&m_root);
	}

	[[nodiscard]] const_iterator before_begin() const noexcept
	{
		return const_iterator(const_cast<detail::forward_list_::hook*>(&m_root));
	}

	[[nodiscard]] iterator begin() noexcept
	{
		return iterator(m_root.next);
	}

	[[nodiscard]] const_iterator begin() const noexcept
	{
		return const_iterator(m_root.next);
	}

	[[nodiscard]] iterator end() noexcept
	{
		return iterator(&m_root);
	}

	[[nodiscard]] const_iterator end() const noexcept
	{
		return const_iterator(const_cast<detail::forward_list_::hook*>(&m_root));
	}


	friend void swap(forward_list& lhs, forward_list& rhs)
	{
		using std::swap;
		swap(static_cast<base&>(lhs), static_cast<base&>(rhs));
	}
};

} // namespace vsm::intrusive
