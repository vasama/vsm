#pragma once

#include <vsm/assert.h>
#include <vsm/atomic.hpp>
#include <vsm/intrusive/forward_list.hpp>

namespace vsm::intrusive {

using mpsc_queue_link = forward_list_link;

namespace detail::mpsc_queue_ {

using hook = detail::forward_list_::hook;

struct hook_pair
{
	hook* head;
	hook* tail;
};

struct base : link_container
{
	atomic<hook_pair> m_atom = hook_pair{ nullptr, nullptr };

	bool push_one(hook* node);
	bool push_all(hook* head, hook* tail);
	hook_pair pop_all_lifo();
	hook_pair pop_all_fifo();
};

} // namespace detail::mpsc_queue_

template<std::derived_from<mpsc_queue_link> T>
class mpsc_queue : detail::mpsc_queue_::base
{
	using base = detail::mpsc_queue_::base;

public:
	mpsc_queue() = default;

	mpsc_queue(mpsc_queue const&) = delete;
	mpsc_queue& operator=(mpsc_queue const&) = delete;


	/// @brief Push an element at the end of the queue.
	/// @returns True if the queue was previously empty.
	/// @pre @p element is not null.
	bool push_back(T* const element)
	{
		vsm_assert(element != nullptr);
		return base::push_one(vsm_detail_forward_list_hook(element));
	}

	/// @brief Splice a list of elements at the end of the queue.
	/// @returns True if the queue was previously empty.
	bool splice_back(forward_list<T>&& list);

	[[nodiscard]] forward_list<T> pop_all()
	{
		auto const pair = base::pop_all_fifo();
		return forward_list<T>(pair.head, pair.tail);
	}

	[[nodiscard]] forward_list<T> pop_all_reversed()
	{
		auto const pair = base::pop_all_lifo();
		return forward_list<T>(pair.head, pair.tail);
	}
};

} // namespace vsm::intrusive
