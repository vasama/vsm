#pragma once

#include <coroutine>

namespace vsm {

template<typename T = void>
class unique_coroutine_handle
{
	std::coroutine_handle<T> m_coroutine;

public:
	constexpr unique_coroutine_handle() noexcept
		: m_coroutine(nullptr)
	{
	}

	constexpr unique_coroutine_handle(decltype(nullptr)) noexcept
		: m_coroutine(nullptr)
	{
	}

	explicit constexpr unique_coroutine_handle(std::coroutine_handle<T> const coroutine) noexcept
		: m_coroutine(coroutine)
	{
	}

	constexpr unique_coroutine_handle(unique_coroutine_handle&& other) noexcept
		: m_coroutine(other.m_coroutine)
	{
		other.m_coroutine = std::coroutine_handle<T>();
	}

	constexpr unique_coroutine_handle& operator=(unique_coroutine_handle&& other) & noexcept
	{
		auto const other_coroutine = other.m_coroutine;
		other.m_coroutine = std::coroutine_handle<T>();

		if (m_coroutine)
		{
			m_coroutine.destroy();
		}

		m_coroutine = other_coroutine;
		return *this;
	}

	constexpr ~unique_coroutine_handle()
	{
		if (m_coroutine)
		{
			m_coroutine.destroy();
		}
	}

	friend constexpr void swap(unique_coroutine_handle& lhs, unique_coroutine_handle& rhs) noexcept
	{
		auto const lhs_coroutine = lhs.m_coroutine;
		lhs.m_coroutine = rhs.m_coroutine;
		rhs.m_coroutine = lhs_coroutine;
	}

	[[nodiscard]] std::coroutine_handle<T> get() const noexcept
	{
		return m_coroutine;
	}

	[[nodiscard]] bool done() const noexcept
	{
		return m_coroutine.done();
	}

	[[nodiscard]] std::coroutine_handle<T> release() noexcept
	{
		auto const coroutine = m_coroutine;
		m_coroutine = std::coroutine_handle<T>();
		return coroutine;
	}

	[[nodiscard]] T& promise() const noexcept
	{
		return m_coroutine.promise();
	}

	void resume() const noexcept
	{
		m_coroutine.resume();
	}

	void reset() noexcept
	{
		auto const coroutine = m_coroutine;
		m_coroutine = std::coroutine_handle<T>();

		if (coroutine)
		{
			coroutine.destroy();
		}
	}

	[[nodiscard]] explicit constexpr operator bool() const noexcept
	{
		return static_cast<bool>(m_coroutine);
	}
};

template<typename T = void>
class symmetric_transfer
{
	std::coroutine_handle<T> m_coroutine;

public:
	explicit symmetric_transfer(std::coroutine_handle<T> const coroutine) noexcept
		: m_coroutine(coroutine)
	{
	}

	std::false_type await_ready() const noexcept
	{
		return {};
	}

	std::coroutine_handle<T> await_suspend(std::coroutine_handle<>) const noexcept
	{
		return m_coroutine;
	}

	void await_resume() const noexcept
	{
	}
};

template<typename T = void>
class symmetric_transfer_and_destroy
{
	std::coroutine_handle<T> m_coroutine;

public:
	explicit symmetric_transfer_and_destroy(std::coroutine_handle<T> const coroutine) noexcept
		: m_coroutine(coroutine)
	{
	}

	std::false_type await_ready() const noexcept
	{
		return {};
	}

	std::coroutine_handle<T> await_suspend(std::coroutine_handle<> const coroutine) const noexcept
	{
		coroutine.destroy();
		return m_coroutine;
	}

	void await_resume() const noexcept
	{
	}
};

} // namespace vsm
