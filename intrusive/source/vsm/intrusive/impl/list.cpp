#include <vsm/intrusive/list.hpp>

using namespace vsm;
using namespace vsm::intrusive;
using namespace vsm::intrusive::detail::list_;

static bool invariant(base const& self)
{
	size_t size = 0;

	hook const* hare = &self.m_root;
	hook const* tortoise = &self.m_root;

	while (true)
	{
		hare = hare->siblings[0]->siblings[0];

		const hook* const next = tortoise->siblings[0];
		if (next->siblings[1] != tortoise)
		{
			return false;
		}
		tortoise = next;

		if (tortoise == hare)
		{
			break;
		}

		++size;
	}

	return size == self.m_size;
}


void base::adopt(hook* const head, size_t const size)
{
	hook* const tail = m_root.siblings[1];
	vsm_assert(tail->siblings[0] == head);

	m_root.siblings[0] = head;
	head->siblings[1] = &m_root;
	
	m_root.siblings[1] = tail;
	tail->siblings[0] = &m_root;

	m_size = size;

	vsm_assert_slow(invariant(*this));
}

void base::insert(hook* const prev, hook* const node, bool const before)
{
	vsm_intrusive_link_insert(*this, *node);

	++m_size;

	hook* const next = prev->siblings[before];

	node->siblings[before] = next;
	node->siblings[!before] = prev;

	prev->siblings[before] = node;
	next->siblings[!before] = node;

	vsm_assert_slow(invariant(*this));
}

void base::remove(hook* const node)
{
	vsm_intrusive_link_remove(*this, *node);

	--m_size;

	hook* const next = node->siblings[0];
	hook* const prev = node->siblings[1];

	prev->siblings[0] = next;
	next->siblings[1] = prev;

	vsm_assert_slow(invariant(*this));
}

void base::clear_internal()
{
	for (hook* node = m_root.children[0]; node != &m_root; node = node->children[0])
	{
		vsm_intrusive_link_remove(*this, *node);
	}
}
