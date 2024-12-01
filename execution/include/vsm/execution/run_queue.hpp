#pragma once

#include <vsm/exceptions.hpp>
#include <vsm/execution/execution.hpp>
#include <vsm/intrusive/mpsc_queue.hpp>
#include <vsm/standard.hpp>

namespace vsm::execution {

#if 0
namespace detail {

template<typename RunQueue>
class _run_queue_ctx
{
public:
	template<typename Condition>
	void run_while(Condition&& condition)
	{
		static_cast<RunQueue&>(*this)._run(vsm_forward(condition));
	}

	[[nodiscard]] typename RunQueue::env get_env() const noexcept
	{
		return typename RunQueue::env(static_cast<RunQueue&>(*this));
	}

private:
	_run_queue_ctx() = default;
	_run_queue_ctx(_run_queue_ctx const&) = default;
	_run_queue_ctx& operator=(_run_queue_ctx const&) = default;
	~_run_queue_ctx() = default;

	friend RunQueue;
};

} // namespace detail
#endif

class run_queue
{
	struct task : intrusive::forward_list_link
	{
		union
		{
			run_queue* m_queue;
			void(* m_execute)(task& task);
		};

		explicit task(run_queue& queue)
			: m_queue(&queue)
		{
		}

		void execute()
		{
			m_execute(*this);
		}
	};

	intrusive::mpsc_queue<task> m_stack;
	intrusive::forward_list<task> m_queue;

	class scheduler;

	class env
	{
		run_queue* m_queue;

	public:
		explicit env(run_queue& queue)
			: m_queue(&queue)
		{
		}

		template<typename CPO>
		[[nodiscard]] scheduler query(std_execution::get_completion_scheduler_t<CPO>) const noexcept
		{
			return m_queue->get_scheduler();
		}
	};

	template<typename Receiver>
	class operation : task
	{
		vsm_no_unique_address Receiver m_receiver;

	public:
		explicit operation(run_queue& queue, auto&& receiver)
			: task(queue)
			, m_receiver(vsm_forward(receiver))
		{
		}

		void start() & noexcept
		{
			run_queue& queue = *m_queue;
			m_execute = execute;
			queue.m_stack.push_back(*this);
		}

	private:
		static void execute(task& task)
		{
			Receiver& receiver = static_cast<operation&>(task).m_receiver;
		
			vsm_except_try
			{
				if (std_execution::get_stop_token(std_execution::get_env(
					static_cast<Receiver const&>(receiver))).stop_requested())
				{
					std_execution::set_stopped(static_cast<Receiver&&>(receiver));
				}
				else
				{
					std_execution::set_value(static_cast<Receiver&&>(receiver));
				}
			}
			vsm_except_catch(...)
			{
				std_execution::set_error(
					static_cast<Receiver&&>(receiver),
					std::current_exception());
			}
		}
	};

	class sender
	{
		run_queue* m_queue;

	public:
		using sender_concept = std_execution::sender_t;
		using completion_signatures = std_execution::completion_signatures<
			std_execution::set_value_t(),
			std_execution::set_stopped_t()>;

		explicit sender(run_queue& queue)
			: m_queue(&queue)
		{
		}

		template<std_execution::receiver Receiver>
		[[nodiscard]] auto connect(Receiver&& receiver) const
		{
			return operation<std::decay_t<Receiver>>(*m_queue, vsm_forward(receiver));
		}

#if 0
		template<std_execution::receiver Receiver>
		[[nodiscard]] friend operation<std::decay_t<Receiver>> tag_invoke(
			std_execution::connect_t,
			sender const& self,
			Receiver&& receiver)
		{
			return operation<std::decay_t<Receiver>>(*self.m_queue, vsm_forward(receiver));
		}
#endif

		[[nodiscard]] env get_env() const noexcept
		{
			return env(*m_queue);
		}
	};

	class scheduler
	{
		run_queue* m_queue;

	public:
		explicit scheduler(run_queue& queue)
			: m_queue(&queue)
		{
		}

		[[nodiscard]] sender schedule() const
		{
			return sender(*m_queue);
		}

		[[nodiscard]] bool query(std_execution::execute_may_block_caller_t) const noexcept
		{
			return false;
		}

		[[nodiscard]] friend bool operator==(scheduler const&, scheduler const&) = default;
	};

public:
	run_queue() = default;

	run_queue(run_queue const&) = delete;
	run_queue& operator=(run_queue const&) = delete;

	[[nodiscard]] scheduler get_scheduler() &
	{
		return scheduler(*this);
	}

	bool run_one() &
	{
		if (m_queue.empty())
		{
			m_queue = m_stack.pop_all();

			if (m_queue.empty())
			{
				return false;
			}
		}

		task& t = m_queue.pop_front();
		t.m_execute(t);

		return true;
	}

	void run_all() &
	{
		while (run_one());
	}
};

} // namespace vsm::execution
