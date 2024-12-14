#include <vsm/intrusive/mpsc_queue.hpp>

using namespace vsm;
using namespace vsm::intrusive;
using namespace vsm::intrusive::detail;

using hook = _mpscq::hook;
static_assert(sizeof(hook) == sizeof(mpsc_queue_link));


static hook* reverse_list(hook* head)
{
	hook* prev = nullptr;

	while (head != nullptr)
	{
		head = std::exchange(head->next, std::exchange(prev, head));
	}

	return prev;
}

bool _mpscq::push_one(hook* const node)
{
	return push_all(node, node);
}

bool _mpscq::push_all(hook* const head, hook* const tail)
{
	auto const update_atom = [&](hook_pair const& atom)
	{
		return hook_pair
		{
			.head = head,
			.tail = atom.tail == nullptr
				? tail
				: atom.tail,
		};
	};

	hook_pair atom = m_atom.load(std::memory_order_acquire);

	do
	{
		tail->next = atom.head;
	}
	while (!m_atom.compare_exchange_weak(
		atom,
		update_atom(atom),
		std::memory_order_release,
		std::memory_order_acquire));

	return atom.head == nullptr;
}

_mpscq::hook_pair _mpscq::pop_all_lifo()
{
	hook_pair pair = m_atom.load(std::memory_order_acquire);

	if (pair.head == nullptr)
	{
		return {};
	}

	pair = m_atom.exchange(hook_pair{}, std::memory_order_acq_rel);

	// Only the consumer can make the head null
	// and the consumer is externally synchronized.
	vsm_assert(pair.head != nullptr);

	return pair;
}

_mpscq::hook_pair _mpscq::pop_all_fifo()
{
	hook_pair const pair = pop_all_lifo();

	hook* const new_head = reverse_list(pair.head);
	vsm_assert(new_head == pair.tail);

	return { new_head, pair.head };
}
