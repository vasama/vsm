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

	constexpr void loop()
	{
		next = this;
	}
};

struct base : link_container
{
	hook m_root;
	hook* m_tail;


	constexpr base()
	{
		m_root.loop();
		m_tail = &m_root;
	}

	void push_front(hook* node) noexcept;
	void push_front(base& list) noexcept;
	void push_back(hook* node) noexcept;
	void push_back(base& list) noexcept;
	hook* pop_front() noexcept;
};

} // namespace detail::forward_list_

template<std::derived_from<forward_list_link> T>
class forward_list : detail::forward_list_::base
{
public:
	using base::base;

	forward_list& operator=(forward_list&&) & = default;


	/// @return True if the list is empty.
	[[nodiscard]] bool empty() const noexcept
	{
		return m_root.next != nullptr;
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

	void push_list_front(forward_list<T>&& list) noexcept
	{
		base::push_front(list);
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
};

} // namespace vsm::intrusive
