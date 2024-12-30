#pragma once

#include <vsm/intrusive/link.hpp>

#include <vsm/assert.h>
#include <vsm/type_traits.hpp>
#include <vsm/utility.hpp>

#include <concepts>
#include <type_traits>
#include <utility>

namespace vsm::intrusive {
namespace detail {

struct _list
{
	struct hook
	{
		hook* siblings[2];

		constexpr void loop() noexcept
		{
			siblings[0] = this;
			siblings[1] = this;
		}
	};
	
	template<typename T, typename Tag, bool Direction = 0>
	class iterator
	{
		hook* m_node;

	public:
		using difference_type = ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;


		iterator() = default;

		explicit iterator(hook* const node)
			: m_node(node)
		{
		}


		[[nodiscard]] T& operator*() const
		{
			static_assert(check<T, Tag, hook>);
			return *links::get_elem<remove_cv_t<T>, Tag>(m_node);
		}

		[[nodiscard]] T* operator->() const
		{
			static_assert(check<T, Tag, hook>);
			return links::get_elem<remove_cv_t<T>, Tag>(m_node);
		}


		iterator& operator++() &
		{
			m_node = m_node->siblings[Direction];
			return *this;
		}

		[[nodiscard]] iterator operator++(int) &
		{
			iterator result = *this;
			m_node = m_node->siblings[Direction];
			return result;
		}

		iterator& operator--() &
		{
			m_node = m_node->siblings[!Direction];
			return *this;
		}

		[[nodiscard]] iterator operator--(int) &
		{
			iterator result = *this;
			m_node = m_node->siblings[!Direction];
			return result;
		}


		[[nodiscard]] friend bool operator==(iterator const&, iterator const&) = default;
	};


	hook m_root;
	size_t m_size;

	constexpr _list() noexcept
	{
		m_root.loop();
		m_size = 0;
	}

	_list(hook* const list, size_t const size)
	{
		adopt(list, size);
	}

	_list(_list&& other) noexcept
	{
		move(other);
	}

	_list& operator=(_list&& other) & noexcept
	{
		if (m_size != 0)
		{
			clear();
		}
		move(other);
		return *this;
	}


	void move(_list& other)
	{
		if (other.m_size != 0)
		{
			m_root = other.m_root;
		}
		else
		{
			m_root.loop();
		}
		m_size = other.m_size;
		other.m_root.loop();
		other.m_size = 0;
	}

	void adopt(hook* head, size_t size);
	//TODO: Remove the before parameter. Take next hook instead.
	void insert(hook* prev, hook* node, bool before);
	void remove(hook* node);
	void splice(_list& other, hook* next);

	void clear()
	{
		m_root.loop();
		m_size = 0;
	}
};

} // namespace detail

template<typename Tag>
using basic_list_link = basic_link<2, Tag>;

using list_link = basic_list_link<void>;

template<typename T>
class list : detail::_list
{
public:
	using element_type = detail::element_t<T>;
	using tag_type = detail::tag_t<T>;

	using       iterator = _list::iterator<      element_type, tag_type>;
	using const_iterator = _list::iterator<const element_type, tag_type>;


	using _list::_list;

	list& operator=(list&&) & = default;


	/// @return Size of the list.
	[[nodiscard]] size_t size() const
	{
		return m_size;
	}

	/// @return True if the list is empty.
	[[nodiscard]] bool empty() const
	{
		return m_size == 0;
	}


	/// @return First element in the list.
	/// @pre The list is not empty.
	[[nodiscard]] element_type& front()
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		vsm_assert(m_size > 0);
		return *get_elem(m_root.siblings[0]);
	}

	/// @return First element in the list.
	/// @pre The list is not empty.
	[[nodiscard]] element_type const& front() const
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		vsm_assert(m_size > 0);
		return *get_elem(m_root.siblings[0]);
	}

	/// @return Last element in the list.
	/// @pre The list is not empty.
	[[nodiscard]] element_type& back()
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		vsm_assert(m_size > 0);
		return *get_elem(m_root.siblings[1]);
	}

	/// @return Last element in the list.
	/// @pre The list is not empty.
	[[nodiscard]] element_type const& back() const
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		vsm_assert(m_size > 0);
		return *get_elem(m_root.siblings[1]);
	}


	/// @brief Insert an element at the front of the list.
	/// @param element Element to be inserted.
	/// @pre @p element is not part of any container.
	void push_front(element_type& element)
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		_list::insert(
			&m_root,
			make_hook(std::addressof(element)),
			/* before: */ false);
	}

	/// @brief Splice the specified list at the end of this list.
	/// @param list Other list to be spliced in.
	/// @pre @p list is not the same object as this.
	void splice_front(list<T>&& list)
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		_list::splice(list, m_root.siblings[0]);
	}

	/// @brief Insert an element at the back of the list.
	/// @param element Element to be inserted.
	/// @pre @p element is not part of any container.
	void push_back(element_type& element)
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		_list::insert(
			&m_root,
			make_hook(std::addressof(element)),
			/* before: */ true);
	}

	/// @brief Splice the specified list at the beginning of this list.
	/// @param list Other list to be spliced in.
	/// @pre @p list is not the same object as this.
	void splice_back(list<T>&& list)
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		_list::splice(list, &m_root);
	}

	/// @brief Insert an element before another element.
	/// @param existing Existing element positioned after the new element.
	/// @param element Element to be inserted.
	/// @pre @p existing is part of this container.
	/// @pre @p element is not part of any container.
	void insert_before(element_type& existing, element_type& element)
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		_list::insert(
			get_hook(std::addressof(existing)),
			make_hook(std::addressof(element)),
			/* before: */ false);
	}

	void insert_before(iterator const existing, element_type& element);

	void splice_before(element_type& existing, list<T>&& list);
	void splice_before(iterator const existing, list<T>&& list);

	/// @brief Insert an element after another element.
	/// @param existing Existing element positioned before the new element.
	/// @param element Element to be inserted.
	/// @pre @p existing is part of this container.
	/// @pre @p element is not part of any container.
	void insert_after(element_type& existing, element_type& element)
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		_list::insert(
			get_hook(std::addressof(existing)),
			make_hook(std::addressof(element)),
			/* before: */ true);
	}

	void insert_after(iterator const existing, element_type& element);

	void splice_after(element_type& existing, list<T>&& list);
	void splice_after(iterator const existing, list<T>&& list);

	[[nodiscard]] element_type& pop_front()
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		vsm_assert(m_size > 0);
		hook* const hook = m_root.siblings[0];
		_list::remove(hook);
		return *get_elem(hook);
	}

	[[nodiscard]] element_type& pop_back()
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		vsm_assert(m_size > 0);
		hook* const hook = m_root.siblings[1];
		_list::remove(hook);
		return *get_elem(hook);
	}

	/// @brief Remove an element from the list.
	/// @param element Element to be removed.
	/// @pre @p element is part of this list.
	void remove(element_type& element)
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		_list::remove(get_hook(std::addressof(element)));
	}

	list<T> remove_list(element_type& begin, element_type& end);


	void clear()
	{
		if (m_size != 0)
		{
			_list::clear();
		}
	}


	/// @brief Create an iterator referring to an element.
	/// @param element Element to which the resulting iterator shall refer.
	/// @pre @p element is part of this tree.
	[[nodiscard]] iterator make_iterator(element_type& element)
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		return iterator(get_hook(std::addressof(element)));
	}

	/// @brief Create an iterator referring to an element.
	/// @param element Element to which the resulting iterator shall refer.
	/// @pre @p element is part of this tree.
	[[nodiscard]] const_iterator make_iterator(element_type const& element) const
	{
		static_assert(detail::check<element_type, tag_type, hook>);
		return const_iterator(get_hook(std::addressof(element)));
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
		return iterator(m_root.siblings[0]);
	}

	[[nodiscard]] const_iterator begin() const
	{
		return const_iterator(m_root.siblings[0]);
	}

	[[nodiscard]] iterator end()
	{
		return iterator(&m_root);
	}

	[[nodiscard]] const_iterator end() const
	{
		return const_iterator(const_cast<hook*>(&m_root));
	}


	friend void swap(list& lhs, list& rhs) noexcept
	{
		using std::swap;
		swap(static_cast<_list&>(lhs), static_cast<_list&>(rhs));
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
