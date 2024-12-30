#pragma once

#include <vsm/assert.h>
#include <vsm/intrusive/link.hpp>

namespace vsm::intrusive {
namespace detail {

struct _flist
{
	struct hook
	{
		hook* next;
	};

	template<typename T, typename Tag>
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


		[[nodiscard]] T& operator*() const
		{
			return *links::get_elem<T, Tag>(m_node);
		}

		[[nodiscard]] T* operator->() const
		{
			return links::get_elem<T, Tag>(m_node);
		}
	
	
		iterator& operator++()
		{
			m_node = m_node->next;
			return *this;
		}

		[[nodiscard]] iterator operator++(int) &
		{
			iterator result = *this;
			m_node = m_node->next;
			return result;
		}


		[[nodiscard]] friend bool operator==(iterator const&, iterator const&) = default;
	};


	hook m_root;
	hook* m_tail;

	constexpr _flist() noexcept
	{
		m_root.next = &m_root;
		m_tail = &m_root;
	}

	explicit _flist(hook* const head, hook* const tail)
	{
		vsm_assert((head != nullptr) == (tail != nullptr));

		if (head != nullptr)
		{
			tail->next = &m_root;
			m_root.next = head;
			m_tail = tail;
		}
		else
		{
			m_root.next = &m_root;
			m_tail = &m_root;
		}
	}

	_flist(_flist&& other) noexcept
	{
		if (other.m_root.next == &other.m_root)
		{
			m_root.next = &m_root;
			m_tail = &m_root;
		}
		else
		{
			other.m_tail->next = &m_root;
			m_root.next = other.m_root.next;
			m_tail = other.m_tail;

			other.m_root.next = &other.m_root;
			other.m_tail = &other.m_root;
		}
	}

	_flist& operator=(_flist&& other) & noexcept
	{
		hook* const head = other.m_root.next;
		hook* const tail = other.m_tail;

		if (head == &other.m_root)
		{
			m_root.next = &m_root;
			m_tail = &m_root;
		}
		else
		{
			other.m_root.next = &other.m_root;
			other.m_tail = &other.m_root;

			tail->next = &m_root;
			m_root.next = head;
			m_tail = tail;
		}

		return *this;
	}


	void push_front(hook* head, hook* tail);
	void push_front(hook* node);
	void splice_front(_flist& list);
	void push_back(hook* head, hook* tail);
	void push_back(hook* node);
	void splice_back(_flist& list);
	hook* pop_front();
	void clear();
};

} // namespace detail

template<typename Tag>
using basic_forward_list_link = basic_link<1, Tag>;

using forward_list_link = basic_forward_list_link<void>;

template<typename T>
class forward_list : detail::_flist
{
public:
	using element_type = detail::element_t<T>;
	using tag_type = detail::tag_t<T>;

	using       iterator = _flist::iterator<      element_type, tag_type>;
	using const_iterator = _flist::iterator<const element_type, tag_type>;


	using _flist::_flist;

	forward_list(forward_list&&) = default;
	forward_list& operator=(forward_list&&) & = default;


	/// @return True if the list is empty.
	[[nodiscard]] bool empty() const
	{
		return m_root.next == &m_root;
	}


	/// @return First element in the list.
	/// @pre The list is not empty.
	[[nodiscard]] element_type& front()
	{
		vsm_assert(m_root.next != &m_root);
		return *get_elem(m_root.next);
	}

	/// @return First element in the list.
	/// @pre The list is not empty.
	[[nodiscard]] element_type const& front() const
	{
		vsm_assert(m_root.next != &m_root);
		return *get_elem(m_root.next);
	}


	/// @brief Insert an element at the front of the list.
	/// @param element Element to be inserted.
	/// @pre @p element is not part of any container.
	void push_front(element_type& element)
	{
		_flist::push_front(make_hook(std::addressof(element)));
	}

	void splice_front(forward_list<T>&& other)
	{
		_flist::splice_front(other);
	}


	void push_back(element_type& element)
	{
		_flist::push_back(make_hook(std::addressof(element)));
	}

	void splice_back(forward_list<T>&& other)
	{
		_flist::splice_back(other);
	}


	/// @brief Remove the first element from the list.
	/// @return The first element in the list.
	/// @pre The list is not empty.
	[[nodiscard]] element_type& pop_front()
	{
		vsm_assert(m_root.next != &m_root);
		return *get_elem(_flist::pop_front());
	}


	void clear()
	{
		if (m_root.next != &m_root)
		{
			_flist::clear();
		}
	}


	[[nodiscard]] iterator before_begin()
	{
		return iterator(&m_root);
	}

	[[nodiscard]] const_iterator before_begin() const
	{
		return const_iterator(const_cast<hook*>(&m_root));
	}

	[[nodiscard]] iterator begin()
	{
		return iterator(m_root.next);
	}

	[[nodiscard]] const_iterator begin() const
	{
		return const_iterator(m_root.next);
	}

	[[nodiscard]] iterator end()
	{
		return iterator(&m_root);
	}

	[[nodiscard]] const_iterator end() const
	{
		return const_iterator(const_cast<hook*>(&m_root));
	}


	friend void swap(forward_list& lhs, forward_list& rhs)
	{
		using std::swap;
		swap(static_cast<_flist&>(lhs), static_cast<_flist&>(rhs));
	}

private:
	[[nodiscard]] static hook* make_hook(element_type* const element)
	{
		return detail::links::construct<hook, tag_type>(element);
	}

	[[nodiscard]] static auto* get_hook(auto* const element)
	{
		return detail::links::get_hook<hook, tag_type>(element);
	}

	[[nodiscard]] static auto* get_elem(auto* const hook)
	{
		return detail::links::get_elem<element_type, tag_type>(hook);
	}
};

} // namespace vsm::intrusive
