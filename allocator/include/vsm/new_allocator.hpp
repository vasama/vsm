#pragma once

#include <vsm/allocator.hpp>

#include <new>

namespace vsm {

class new_allocator
{
public:
	allocation allocate(size_t const size)
	{
		return { operator new(size), size };
	}
	
	void deallocate(allocation const allocation)
	{
		operator delete(allocation.buffer, allocation.size);
	}
};

} // namespace vsm
