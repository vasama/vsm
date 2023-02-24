#include <vsm/execution/async_mutex.hpp>

#include <vsm/assert.h>

using namespace vsm;
using namespace vsm::execution;

void async_mutex::unlock() & noexcept
{
	vsm_assert(m_state.load(std::memory_order_relaxed) != unlocked_state());

	operation_base* queue = m_queue;

	if (queue == nullptr)
	{
		void* state = m_state.load(std::memory_order_relaxed);

		if (state == nullptr &&
			m_state.compare_exchange_strong(
				state,
				unlocked_state(),
				std::memory_order_release,
				std::memory_order_relaxed))
		{
			return;
		}

		state = m_state.exchange(nullptr, std::memory_order_acquire);
		vsm_assert(state != nullptr && state != unlocked_state());

		operation_base* new_queue = static_cast<operation_base*>(state);

		do
		{
			operation_base* const next = new_queue->next;
			new_queue->next = queue;
			queue = new_queue;
			new_queue = next;
		}
		while (new_queue != nullptr);
	}

	m_queue = queue->next;
	queue->signal(*this, *queue);
}

bool async_mutex::_lock(operation_base& operation)
{
	void* old_state = m_state.load(std::memory_order_relaxed);

	while (true)
	{
		if (old_state == unlocked_state())
		{
			void* new_state = nullptr;
			if (m_state.compare_exchange_weak(
				old_state,
				new_state,
				std::memory_order_acquire,
				std::memory_order_relaxed))
			{
				return true;
			}
		}
		else
		{
			operation->next = static_cast<operation_base*>(old_state);

			void* new_state = &operation;
			if (m_state.compare_exchange_weak(
				old_state,
				new_state,
				std::memory_order_release,
				std::memory_order_relaxed))
			{
				return false;
			}
		}
	}
}
