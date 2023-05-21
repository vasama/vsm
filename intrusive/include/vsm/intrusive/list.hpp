#pragma once

#include <vsm/intrusive/link.hpp>

#include <vsm/assert.h>
#include <vsm/utility.hpp>

#include <concepts>
#include <type_traits>
#include <utility>

namespace vsm::intrusive {

using list_link = link<2>;

namespace detail::list_ {

#define vsm_detail_list_hook(element) \
	(reinterpret_cast<list_::hook*>(static_cast<list_link*>(element)))

#define vsm_detail_list_elem(hook) \
	(static_cast<T*>(reinterpret_cast<list_link*>(hook)))


struct hook : link_base
{
	hook* siblings[2];

	constexpr void loop() noexcept
	{
		siblings[0] = this;
		siblings[1] = this;
	}
};

struct base : link_container
{
	hook m_root;
	size_t m_size;

	constexpr base()
	{
		m_root.loop();
		m_size = 0;
	}

	base(link_container&& container, hook* const list, size_t const size) noexcept
		: link_container(vsm_move(container))
	{
		adopt(list, size);
	}

	base(base&& other) noexcept
		: link_container(static_cast<link_container&&>(other))
	{
		move_init(vsm_move(other));
	}

	base& operator=(base&& other) & noexcept
	{
		if (m_size != 0)
		{
			clear();
		}
		static_cast<link_container&>(*this) = static_cast<link_container&&>(other);
		move_init(vsm_move(other));
		return *this;
	}

	void move_init(base&& other) noexcept
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
	void insert(hook* prev, hook* node, bool before);
	void remove(hook* node);

	void clear()
	{
		clear_internal();
		m_root.loop();
		m_size = 0;
	}

	void clear_internal();
};


class iterator_base
{
protected:
	hook* m_node;

public:
	iterator_base() = default;

	iterator_base(hook* const node) noexcept
		: m_node(node)
	{
	}
};

template<typename T, bool Direction = 0>
class iterator : public iterator_base
{
public:
	using difference_type = ptrdiff_t;
	using value_type = T;
	using pointer = T*;
	using reference = T&;


	using iterator_base::iterator_base;


	[[nodiscard]] T& operator*() const noexcept
	{
		return *vsm_detail_list_elem(m_node);
	}

	[[nodiscard]] T* operator->() const noexcept
	{
		return vsm_detail_list_elem(m_node);
	}


	iterator& operator++() & noexcept
	{
		m_node = m_node->siblings[Direction];
		return *this;
	}

	[[nodiscard]] iterator operator++(int) & noexcept
	{
		iterator result = *this;
		m_node = m_node->siblings[Direction];
		return result;
	}

	iterator& operator--() & noexcept
	{
		m_node = m_node->siblings[!Direction];
		return *this;
	}

	[[nodiscard]] iterator operator--(int) & noexcept
	{
		iterator result = *this;
		m_node = m_node->siblings[!Direction];
		return result;
	}


	[[nodiscard]] friend bool operator==(iterator const& lhs, iterator const& rhs)
	{
		return lhs.m_node == rhs.m_node;
	}
};


template<std::derived_from<list_link> T>
class list : base
{
public:
	using       iterator = list_::iterator<      T>;
	using const_iterator = list_::iterator<const T>;


	using base::base;

	list& operator=(list&&) & = default;


	/// @return Size of the list.
	[[nodiscard]] size_t size() const noexcept
	{
		return m_size;
	}

	/// @return True if the list is empty.
	[[nodiscard]] bool empty() const noexcept
	{
		return m_size == 0;
	}


	/// @return First element in the list.
	/// @pre The list is not empty.
	[[nodiscard]] T* front() noexcept
	{
		vsm_assert(m_size > 0);
		return vsm_detail_list_elem(m_root.siblings[0]);
	}

	/// @return First element in the list.
	/// @pre The list is not empty.
	[[nodiscard]] T const* front() const noexcept
	{
		vsm_assert(m_size > 0);
		return vsm_detail_list_elem(m_root.siblings[0]);
	}

	/// @return Last element in the list.
	/// @pre The list is not empty.
	[[nodiscard]] T* back() noexcept
	{
		vsm_assert(m_size > 0);
		return vsm_detail_list_elem(m_root.siblings[1]);
	}

	/// @return Last element in the list.
	/// @pre The list is not empty.
	[[nodiscard]] T const* back() const noexcept
	{
		vsm_assert(m_size > 0);
		return vsm_detail_list_elem(m_root.siblings[1]);
	}



	/// @brief Insert an element at the front of the list.
	/// @param element Element to be inserted.
	/// @pre @p element is not part of any container.
	void push_front(T* const element) noexcept
	{
		base::insert(&m_root, vsm_detail_list_hook(element), 0);
	}

	void push_list_front(list<T>&& list) noexcept;

	/// @brief Insert an element at the back of the list.
	/// @param element Element to be inserted.
	/// @pre @p element is not part of any container.
	void push_back(T* const element) noexcept
	{
		base::insert(&m_root, vsm_detail_list_hook(element), 1);
	}

	void push_list_back(list<T>&& list) noexcept;

	/// @brief Insert an element before another element.
	/// @param existing Existing element positioned after the new element.
	/// @param element Element to be inserted.
	/// @pre @p existing is part of this container.
	/// @pre @p element is not part of any container.
	void insert_before(T* const existing, T* const element) noexcept
	{
		base::insert(vsm_detail_list_hook(existing), vsm_detail_list_hook(element), 0);
	}

	void insert_list_before(T* const existing, list<T>&& list) noexcept;

	/// @brief Insert an element after another element.
	/// @param existing Existing element positioned before the new element.
	/// @param element Element to be inserted.
	/// @pre @p existing is part of this container.
	/// @pre @p element is not part of any container.
	void insert_after(T* const existing, T* const element) noexcept
	{
		base::insert(vsm_detail_list_hook(existing), vsm_detail_list_hook(element), 1);
	}

	void insert_list_after(T* const existing, list<T>&& list) noexcept;

	/// @brief Remove an element from the list.
	/// @param element Element to be removed.
	/// @pre @p element is part of this list.
	void remove(T* const element) noexcept
	{
		base::remove(vsm_detail_list_hook(element));
	}

	list<T> remove_list(T* const begin, T* const end) noexcept;


	void clear() noexcept
	{
		if (m_size != 0)
		{
			base::clear();
		}
	}


	[[nodiscard]] iterator begin() noexcept
	{
		return iterator(m_root.siblings[0]);
	}

	[[nodiscard]] const_iterator begin() const noexcept
	{
		return const_iterator(m_root.siblings[0]);
	}

	[[nodiscard]] iterator end() noexcept
	{
		return iterator(&m_root);
	}

	[[nodiscard]] const_iterator end() const noexcept
	{
		return const_iterator(const_cast<hook*>(&m_root));
	}


	friend void swap(list& lhs, list& rhs)
	{
		using std::swap;
		swap(static_cast<base&>(lhs), static_cast<base&>(rhs));
	}
};

} // namespace detail::list_

using detail::list_::list;

} // namespace vsm::intrusive
