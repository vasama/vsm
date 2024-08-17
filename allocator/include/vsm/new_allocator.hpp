#pragma once

#include <vsm/allocator.hpp>

#include <new>

namespace vsm {

class new_allocator
{
public:
	static constexpr bool is_always_equal = true;
	static constexpr bool is_propagatable = true;

	[[nodiscard]] allocation allocate(size_t const size) const
	{
		return { operator new(size), size };
	}

	void deallocate(allocation const allocation) const
	{
		operator delete(allocation.buffer, allocation.size);
	}
};

} // namespace vsm
