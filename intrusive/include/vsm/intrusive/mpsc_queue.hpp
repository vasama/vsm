#pragma once

#include <vsm/assert.h>
#include <vsm/atomic.hpp>
#include <vsm/intrusive/forward_list.hpp>

namespace vsm::intrusive {
namespace detail {

struct _mpscq
{
	using hook = _flist::hook;

	struct hook_pair
	{
		hook* head;
		hook* tail;
	};

	atomic<hook_pair> m_atom = hook_pair{ nullptr, nullptr };

	bool push_one(hook* node);
	bool push_all(hook* head, hook* tail);
	hook_pair pop_all_lifo();
	hook_pair pop_all_fifo();
};

} // namespace detail

template<typename Tag>
using basic_mpsc_queue_link = basic_forward_list_link<Tag>;

using mpsc_queue_link = basic_mpsc_queue_link<void>;

template<typename T>
class mpsc_queue : detail::_mpscq
{
public:
	using element_type = detail::element_t<T>;
	using tag_type = detail::tag_t<T>;


	mpsc_queue() = default;

	mpsc_queue(mpsc_queue const&) = delete;
	mpsc_queue& operator=(mpsc_queue const&) = delete;


	/// @brief Push an element at the end of the queue.
	/// @returns True if the queue was previously empty.
	/// @pre @p element is not null.
	bool push_back(element_type* const element)
	{
		vsm_assert(element != nullptr);
		return _mpscq::push_one(detail::links::construct<hook, tag_type>(element));
	}

	/// @brief Splice a list of elements at the end of the queue.
	/// @returns True if the queue was previously empty.
	bool splice_back(forward_list<T>&& list);

	[[nodiscard]] forward_list<T> pop_all()
	{
		auto const pair = _mpscq::pop_all_fifo();
		return forward_list<T>(pair.head, pair.tail);
	}

	[[nodiscard]] forward_list<T> pop_all_reversed()
	{
		auto const pair = _mpscq::pop_all_lifo();
		return forward_list<T>(pair.head, pair.tail);
	}
};

} // namespace vsm::intrusive
