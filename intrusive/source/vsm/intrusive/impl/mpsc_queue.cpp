#include <vsm/intrusive/mpsc_queue.hpp>

using namespace vsm;
using namespace vsm::intrusive;
using namespace vsm::intrusive::detail::mpsc_queue_;


static hook* reverse_list(hook* head)
{
	hook* prev = nullptr;

	while (head != nullptr)
	{
		head = std::exchange(head->next, std::exchange(prev, head));
	}

	return prev;
}


bool base::push_one(hook* const node)
{
	return push_all(node, node);
}

bool base::push_all(hook* const head, hook* const tail)
{
	hook* expected = m_produce_head.load(std::memory_order_acquire);

	do
	{
		tail->next = expected;
	}
	while (!m_produce_head.compare_exchange_weak(
		expected, head,
		std::memory_order_release, std::memory_order_acquire));

	return expected == nullptr;
}

hook_pair base::pop_all()
{
	if (m_produce_head.load(std::memory_order_acquire) == nullptr)
	{
		return {};
	}

	hook* head = m_produce_head.exchange(nullptr, std::memory_order_acq_rel);
	
	// Only the consumer can make the head null and the consumer is externally synchronized.
	vsm_assert(head != nullptr);

	return { reverse_list(head), head };
}
