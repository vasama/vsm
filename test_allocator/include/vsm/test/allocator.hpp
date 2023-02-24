#pragma once

#include <vsm/allocator.hpp>

namespace vsm::test {
namespace detail {

allocation allocate(size_t size);
void deallocate(allocation allocation);

struct allocation_scope_options
{
	bool reversed = false;
	bool maximized = false;
};

void* enter_allocation_scope(allocation_scope_options const& options);
void leave_allocation_scope(void* scope);

size_t get_scope_allocation_count(void* scope);

} // namespace detail

class allocator
{
public:
	static constexpr bool is_always_equal = true;
	static constexpr bool is_propagatable = true;

	[[nodiscard]] allocation allocate(size_t const size) const
	{
		return detail::allocate(size);
	}

	void deallocate(allocation const allocation) const
	{
		detail::deallocate(allocation);
	}
};

class allocation_scope
{
	void* m_scope;

public:
	allocation_scope(detail::allocation_scope_options const& options = {})
		: m_scope(detail::enter_allocation_scope(options))
	{
	}

	allocation_scope(allocation_scope const&) = delete;
	allocation_scope& operator=(allocation_scope const&) = delete;

	~allocation_scope()
	{
		detail::leave_allocation_scope(m_scope);
	}


	[[nodiscard]] size_t get_allocation_count() const
	{
		return detail::get_scope_allocation_count(m_scope);
	}
};

} // namespace vsm::test
