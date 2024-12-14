#include <vsm/intrusive/forward_list.hpp>

using namespace vsm;
using namespace vsm::intrusive;
using namespace vsm::intrusive::detail;

using hook = _flist::hook;
static_assert(sizeof(hook) == sizeof(forward_list_link));


void _flist::push_front(hook* const head, hook* const tail)
{
	if (m_tail == &m_root)
	{
		m_tail = tail;
	}

	tail->next = m_root.next;
	m_root.next = head;
}

void _flist::push_front(hook* const node)
{
	push_front(node, node);
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

	return head;
}

void _flist::clear()
{
	m_root.next = &m_root;
	m_tail = &m_root;
}
