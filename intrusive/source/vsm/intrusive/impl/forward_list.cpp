#include <vsm/intrusive/forward_list.hpp>

using namespace vsm;
using namespace vsm::intrusive;
using namespace vsm::intrusive::detail;

using hook = _flist::hook;
static_assert(sizeof(hook) == sizeof(forward_list_link));


[[maybe_unused]] static bool invariant(_flist const& self)
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

void _flist::push_front(hook* const head, hook* const tail)
{
	if (m_tail == &m_root)
	{
		m_tail = tail;
	}

	tail->next = m_root.next;
	m_root.next = head;

	//vsm_assert_slow(invariant(*this));
}

void _flist::push_front(hook* const node)
{
	return push_front(node, node);
}

void _flist::splice_front(_flist& list)
{
	if (list.m_root.next != &list.m_root)
	{
		push_front(list.m_root.next, list.m_tail);
		list.clear();
	}
}

void _flist::push_back(hook* const head, hook* const tail)
{
	tail->next = &m_root;
	m_tail->next = head;
	m_tail = tail;

	//vsm_assert_slow(invariant(*this));
}

void _flist::push_back(hook* const node)
{
	push_back(node, node);
}

void _flist::splice_back(_flist& list)
{
	if (list.m_root.next != &list.m_root)
	{
		push_back(list.m_root.next, list.m_tail);
		list.clear();
	}
}

hook* _flist::pop_front()
{
	hook* const head = m_root.next;
	m_root.next = head->next;

	if (m_tail == head)
	{
		m_tail = &m_root;
	}

	//vsm_assert_slow(invariant(*this));

	return head;
}

void _flist::clear()
{
	m_root.next = &m_root;
	m_tail = &m_root;
}
