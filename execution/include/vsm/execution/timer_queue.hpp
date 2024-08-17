#pragma once

#include <vsm/exceptions.hpp>
#include <vsm/execution/execution.hpp>
#include <vsm/intrusive/heap.hpp>
#include <vsm/standard.hpp>

namespace vsm::execution {

template<typename Clock>
class timer_queue
{
	class task : intrusive::heap_link
	{
		Clock::time_point m_time;
		union {
			timer_queue* m_queue;
			void(* m_execute)(task& task);
		};
	};

	intrusive::heap<task> m_heap;
	intrusive::mpsc_queue<task> m_mpsc;

	template<typename Receiver>
	class operation : task
	{
		vsm_no_unique_address Receiver m_receiver;

	public:
		explicit operation(timer_queue& queue, auto&& receiver)
			: task(queue)
			, m_receiver(vsm_forward(receiver))
		{
		}

		void start() & noexcept
		{
			auto& queue = *m_queue;
			m_execute = execute;
		}

	private:
		static void execute(task& task)
		{
			
		}
	};

public:
	timer_queue() = delete;

	timer_queue(timer_queue const&) = delete;
	timer_queue& operator=(timer_queue const&) = delete;

	
};

} // namespace vsm::execution
