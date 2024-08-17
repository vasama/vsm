#pragma once

#include <vsm/atomic.hpp>
#include <vsm/execution/execution.hpp>
#include <vsm/standard.hpp>
#include <vsm/tag_ptr.hpp>

namespace vsm::execution {

class async_mutex
{
	struct operation_base
	{
		union
		{
			async_mutex* m_mutex;
			operation_base* m_next;
		};
		void(* m_continuation)(async_mutex& mutex, operation_base& operation);
	};

	atomic<void*> m_state;
	operation_base* m_queue;

	template<typename... Result>
	class lock_sender
	{
		async_mutex* m_mutex;

		template<typename Receiver>
		class operation : operation_base
		{
			vsm_no_unique_address Receiver m_receiver;

		public:
			explicit operation(async_mutex& mutex, vsm::any_cvref_of<Receiver> auto&& receiver)
				: operation_base{ &mutex }
				, m_receiver(vsm_forward(receiver))
			{
			}

			void start() & noexcept
			{
				async_mutex& mutex = *m_mutex;
				m_continuation = continuation;

				if (mutex._lock(*this))
				{
					std_execution::set_value(vsm_move(m_receiver), Result(mutex)...);
				}
			}

		private:
			static void continuation(async_mutex& mutex, operation_base& self)
			{
				std_execution::set_value(static_cast<operation&&>(self).m_receiver, Result(mutex)...);
			}
		};

	public:
		using completion_signatures = std_execution::completion_signatures<
			std_execution::set_value(Result...)>;

		explicit lock_sender(async_mutex& mutex)
			: m_mutex(&mutex)
		{
		}

		template<std_execution::receiver Receiver>
		[[nodiscard]] operation<std::decay_t<Receiver>> connect(Receiver&& receiver) const
		{
			return operation<std::decay_t<Receiver>>(*m_mutex, vsm_forward(receiver));
		}
	};

public:
	async_mutex()
		: m_state(unlocked_state())
		, m_queue(nullptr)
	{
	}

	async_mutex(async_mutex const&) = delete;
	async_mutex& operator=(async_mutex const&) = delete;

	~async_mutex()
	{
		vsm_assert(
			m_state.load(std::memory_order_relaxed) == unlocked_state() &&
			m_queue == nullptr);
	}

	[[nodiscard]] bool try_lock() & noexcept
	{
		void* expected_state = unlocked_state();
		return m_state.compare_exchange_strong(
			expected_state,
			/* new_state: */ nullptr,
			std::memory_order_acq_rel,
			std::memory_order_release);
	}

	void unlock() & noexcept;

	[[nodiscard]] lock_sender<> lock() &
	{
		return lock_sender<>(*this);
	}

	[[nodiscard]] lock_sender<std::unique_lock<async_mutex>> scoped_lock() &
	{
		return lock_sender<std::unique_lock<async_mutex>>(*this);
	}

private:
	[[nodiscard]] void* unlocked_state()
	{
		return this;
	}

	[[nodiscard]] bool _lock(operation_base& operation);
};

class async_shared_mutex
{
public:
	
};

} // namespace vsm::execution
