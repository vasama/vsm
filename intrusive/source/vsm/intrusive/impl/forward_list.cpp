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

void base::push_front(hook* const node) noexcept
{
	if (m_tail == &m_root)
	{
		m_tail = node;
	}

	node->next = m_root.next;
	m_root.next = node;

	vsm_assert_slow(invariant(*this));
}

void base::push_back(hook* const node) noexcept
{
	m_tail->next = node;
	m_tail = node;

	vsm_assert_slow(invariant(*this));
}

hook* base::pop_front() noexcept
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
