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
	atomic<hook*> m_produce_head = nullptr;

	void push_one(hook* node) noexcept;
	void push_all(hook* head, hook* tail) noexcept;
	hook_pair pop_all() noexcept;
};

} // namespace detail::mpsc_queue_

template<std::derived_from<mpsc_queue_link> T>
class mpsc_queue : detail::mpsc_queue_::base
{
public:
	using base::base;

	mpsc_queue& operator=(mpsc_queue&&) & = default;


	/// @pre @p element is not null.
	void push(T* const element) noexcept
	{
		vsm_assert(element != nullptr);
		base::push_one(vsm_detail_forward_list_hook(element));
	}

	void push_list(forward_list<T>&& list) noexcept;

	[[nodiscard]] forward_list<T> pop_all() noexcept
	{
		auto const pair = base::pop_all();
		return forward_list<T>(pair.head, pair.tail);
	}
};

} // namespace vsm::intrusive
