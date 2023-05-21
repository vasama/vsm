#include <vsm/intrusive/forward_list.hpp>

using namespace vsm;
using namespace vsm::intrusive;
using namespace vsm::intrusive::detail::forward_list_;

static_assert(sizeof(hook) == sizeof(forward_list_link));


static bool invariant(base const& self)
{
	hook const* hare = self.m_root.next;
	hook const* tortoise = hare;

	hook const* const tail = self.m_tail;

	if (hare == nullptr)
	{
		return tail == nullptr;
	}

	while (true)
	{
		if ((hare = hare->next) == nullptr)
		{
			return true;
		}

		if ((hare = hare->next) == nullptr)
		{
			return true;
		}

		tortoise = tortoise->next;
		vsm_assert(tortoise != nullptr);

		if (tortoise == hare)
		{
			return false;
		}
	}
}

void base::insert(hook* const node)
{
	node->next = m_root.next;
	m_root.next = node;

	if (m_tail == nullptr)
	{
		m_tail = node;
	}

	vsm_assert_slow(invariant(*this));
}
