#pragma once

#include <vsm/intrusive/heap.hpp>

namespace vsm {

#if 0
template<typename TimePoint>
class async_timer
{
	TimePoint m_time_point;

public:
	virtual void timer_completed(TimePoint const& time) = 0;
	virtual void timer_abandoned() = 0;

private:
	friend class async_timer_queue<TimePoint>;
	friend class vsm::intrusive::min_heap<async_timer>;
};

template<typename TimePoint>
class async_timer_queue
{
	using async_timer_type = async_timer<TimePoint>;

	vsm::intrusive::min_heap<async_timer_type> m_timer_heap;

	class delay_sender
	{
		async_timer_queue* m_timer_queue;
		TimePoint m_time_point;

		template<typename Receiver>
		class operation : async_timer_type
		{
			async_timer_queue* m_timer_queue;
			vsm_no_unique_address Receiver m_receiver;

		public:
			operation(auto&& sender, auto&& receiver)
				: m_timer_queue(sender.m_timer_queue)
				, m_receiver(vsm_forward(receiver))
			{
				m_time_point = vsm_forward(sender).m_time_point;
			}

			void start() & noexcept
			{
				m_timer_queue->m_timer_heap.push(this);
			}

		private:
			void timer_completed(TimePoint const& time) override
			{
				set_value(vsm_move(m_receiver), time);
			}
			
			void timer_abandoned() override
			{
				set_done(vsm_move(m_receiver));
			}
		};

	public:
		template<typename Receiver>
		operation<std::remove_reference_t<Receiver>> connect(Receiver&& receiver)
		{
			return { *this, vsm_forward(receiver) };
		}
	};

public:
	async_timer_queue() = default;

	async_timer_queue(async_timer_queue&&) = default;
	async_timer_queue& operator=(async_timer_queue&&) & = default;

	~async_timer_queue()
	{
		cancel_all();
	}


	[[nodiscard]] bool empty() const
	{
		return m_timer_heap.empty();
	}

	[[nodiscard]] async_timer_type const& next_timer() const
	{
		return *m_timer_heap.peek();
	}

	void schedule(async_timer_type& timer, TimePoint time)
	{
		timer.m_time_point = time;
		m_timer_heap.push(&timer);
	}

	void cancel(async_timer_type& timer)
	{
		m_timer_heap.remove(&timer);
		timer.timer_abandoned();
	}

	void cancel_all()
	{
		while (!m_timer_heap.empty())
		{
			m_timer_heap.pop()->timer_abandoned();
		}
	}


	size_t expire(size_t const max = static_cast<size_t>(-1))
	{
		return expire(TimePoint::clock::now(), max);
	}

	size_t expire(TimePoint const time, size_t const max = static_cast<size_t>(-1))
	{
		size_t count = 0;
		for (; count < max && !m_timer_heap.empty() && time >= m_timer_heap.peek()->m_time_point; ++count)
		{
			m_timer_heap.pop()->timer_ended(time);
		}
		return count;
	}

	size_t expire_one()
	{
		return expire(1zu);
	}

	size_t expire_one(TimePoint const time)
	{
		return expire(time, 1zu);
	}


	delay_sender delay_until(TimePoint time_point)
	{
		return delay_sender(*this, time_point);
	}

	delay_sender delay(TimePoint::duration duration)
	{
		return delay_sender(*this, TimePoint::clock::now() + vsm_move(duration));
	}
};
#endif


template<typename Timer>
class async_timer_queue;

class async_timer : vsm::intrusive::heap_link
{
	template<typename Timer>
	friend class async_timer_queue;

	template<typename, typename, typename>
	friend class intrusive::heap;
};

template<typename Timer>
class async_timer_queue
{
	static_assert(std::derived_from<Timer, async_timer>);

public:
	using time_point = typename Timer::time_point;
	using duration = typename time_point::duration;

private:
	struct key_selector
	{
		decltype(auto) vsm_static_operator_invoke(Timer const& timer)
		{
			return timer.get_time_point();
		}
	};

	vsm::intrusive::min_heap<Timer, key_selector> m_timer_heap;


	struct sender
	{
	};

public:
	async_timer_queue() = default;

	async_timer_queue(async_timer_queue&&) = default;
	async_timer_queue& operator=(async_timer_queue&&) & = default;

	~async_timer_queue()
	{
		cancel_all();
	}


	[[nodiscard]] bool empty() const
	{
		return m_timer_heap.empty();
	}

	[[nodiscard]] Timer const& next_timer() const
	{
		return *m_timer_heap.peek();
	}


	void schedule(Timer& timer, time_point time)
	{
		timer.set_time_point(vsm_move(time));
		m_timer_heap.push(&timer);
	}

	void cancel(Timer& timer)
	{
		m_timer_heap.remove(&timer);
		timer.timer_abandoned();
	}

	void cancel_all()
	{
		while (!m_timer_heap.empty())
		{
			m_timer_heap.pop()->timer_abandoned();
		}
	}


	size_t expire(size_t const max = static_cast<size_t>(-1))
	{
		return expire_internal(time_point::clock::now(), max);
	}

	size_t expire(time_point const now, size_t const max = static_cast<size_t>(-1))
	{
		return expire_internal(now, max);
	}

	size_t expire_one()
	{
		return expire_internal(time_point::clock::now(), 1zu);
	}

	size_t expire_one(time_point const now)
	{
		return expire_internal(now, 1zu);
	}

private:
	size_t expire_internal(time_point const now, size_t const max)
	{
		size_t count = 0;
		while (count < max && !m_timer_heap.empty() && now >= m_timer_heap.peek()->get_time_point())
		{
			m_timer_heap.pop()->timer_ended(now);
			++count;
		}
		return count;
	}
};

} // namespace vsm
