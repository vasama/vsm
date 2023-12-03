#include <vsm/intrusive/forward_list.hpp>

using namespace vsm;
using namespace vsm::intrusive;
using namespace vsm::intrusive::detail::forward_list_;

static_assert(sizeof(hook) == sizeof(forward_list_link));


static bool invariant(base const& self)
{
	hook const* hare = self.m_root.next;
	hook const* tortoise = hare;

	while (true)
	{
		if (hare->next == &self.m_root)
		{
			break;
		}
		hare = hare->next;

		if (hare->next == &self.m_root)
		{
			break;
		}
		hare = hare->next;

		tortoise = tortoise->next;
		vsm_assert(tortoise != &self.m_root);

		if (tortoise == hare)
		{
			return false;
		}
	}

	return hare == self.m_tail;
}

void base::push_front(hook* const head, hook* const tail)
{
	if (m_tail == &m_root)
	{
		m_tail = tail;
	}

	tail->next = m_root.next;
	m_root.next = head;

	vsm_assert_slow(invariant(*this));
}

void base::push_front(hook* const node)
{
	return push_front(node, node);
}

void base::splice_front(base& list)
{
	if (list.m_root.next != &list.m_root)
	{
		push_front(list.m_root.next, list.m_tail);
		list.clear();
	}
}

void base::push_back(hook* const head, hook* const tail)
{
	tail->next = &m_root;
	m_tail->next = head;
	m_tail = tail;

	vsm_assert_slow(invariant(*this));
}

void base::push_back(hook* const node)
{
	push_back(node, node);
}

void base::splice_back(base& list)
{
	if (list.m_root.next != &list.m_root)
	{
		push_back(list.m_root.next, list.m_tail);
		list.clear();
	}
}

hook* base::pop_front()
{
	hook* const head = m_root.next;
	m_root.next = head->next;

	if (m_tail == head)
	{
		m_tail = &m_root;
	}

	vsm_assert_slow(invariant(*this));

	return head;
}

void base::clear()
{
	m_root.next = &m_root;
	m_tail = &m_root;
}
