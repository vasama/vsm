#pragma once

#include <utility>

#include <cstddef>
#include <cstdint>

namespace vsm::intrusive {
namespace detail {

using link_shared = uintptr_t;

void release_link_shared(link_shared shared);

struct link_shared_handle
{
	link_shared m_shared = 0;

	link_shared_handle() = default;

	link_shared_handle(link_shared_handle&& src) noexcept
		: m_shared(src.m_shared)
	{
		src.m_shared = 0;
	}

	link_shared_handle& operator=(link_shared_handle&& src) & noexcept
	{
		if (m_shared != 0)
		{
			release_link_shared(m_shared);
		}
		m_shared = src.m_shared;
		src.m_shared = 0;
		return *this;
	}

	~link_shared_handle()
	{
		if (m_shared != 0)
		{
			release_link_shared(m_shared);
		}
	}

	friend void swap(link_shared_handle& lhs, link_shared_handle& rhs) noexcept
	{
		std::swap(lhs.m_shared, rhs.m_shared);
	}
};

class link_base
{
};

class link_container
{
	friend void vsm_intrusive_link_insert(link_container& container, link_base& link) noexcept
	{
	}

	friend void vsm_intrusive_link_remove(link_container& container, link_base& link) noexcept
	{
	}

	friend void vsm_intrusive_link_check(link_container const& container, link_base const& link) noexcept
	{
	}
};

} // namespace detail

template<size_t Size> requires (Size > 0)
class link : public link<Size - 1>
{
	[[maybe_unused]] uintptr_t data;
};

template<>
class link<1> : public detail::link_base
{
	[[maybe_unused]] uintptr_t data;
};

} // namespace vsm::intrusive
