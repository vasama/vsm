#include <vsm/intrusive/list.hpp>

using namespace vsm;
using namespace vsm::intrusive;
using namespace vsm::intrusive::detail;

using hook = _list::hook;

static_assert(sizeof(hook) == sizeof(list_link));
static_assert(std::is_standard_layout_v<hook>);


void _list::adopt(hook* const head, size_t const size)
{
	hook* const tail = m_root.siblings[1];
	vsm_assert(tail->siblings[0] == head);

	m_root.siblings[0] = head;
	head->siblings[1] = &m_root;

	m_root.siblings[1] = tail;
	tail->siblings[0] = &m_root;

	m_size = size;
}

void _list::insert(hook* const prev, hook* const node, bool const before)
{
	++m_size;

	hook* const next = prev->siblings[before];

	node->siblings[before] = next;
	node->siblings[!before] = prev;

	prev->siblings[before] = node;
	next->siblings[!before] = node;
}

void _list::remove(hook* const node)
{
	--m_size;

	hook* const next = node->siblings[0];
	hook* const prev = node->siblings[1];

	prev->siblings[0] = next;
	next->siblings[1] = prev;
}

void _list::splice(_list& other, hook* const next)
{
	if (other.m_size == 0)
	{
		return;
	}

	hook* const prev = next->siblings[1];
	hook* const head = other.m_root.siblings[0];
	hook* const tail = other.m_root.siblings[1];

	tail->siblings[0] = next;
	next->siblings[1] = tail;

	head->siblings[1] = prev;
	prev->siblings[0] = head;

	other.m_root.loop();
	m_size += other.m_size;
	other.m_size = 0;
}
