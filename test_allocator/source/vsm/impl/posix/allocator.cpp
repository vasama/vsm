#include <vsm/impl/allocator.hpp>

#include <cstdlib>

#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>

using namespace vsm;
using namespace vsm::test;

size_t test::virtual_page_size()
{
	return static_cast<size_t>(getpagesize());
}

void* test::virtual_allocate(size_t const size)
{
	void* const address = mmap(
		/* addr: */ nullptr,
		size,
		PROT_NONE,
		MAP_PRIVATE | MAP_ANONYMOUS,
		/* file: */ -1,
		/* offset: */ 0);

	if (address == MAP_FAILED)
	{
		[[maybe_unused]] int const error = errno;
		std::abort();
	}

	return address;
}

void test::virtual_deallocate(void* const address, size_t const size)
{
	if (munmap(address, size) == -1)
	{
		[[maybe_unused]] int const error = errno;
		std::abort();
	}
}

void test::virtual_commit(void* const address, size_t const size)
{
	if (mprotect(address, size, PROT_READ | PROT_WRITE) == -1)
	{
		[[maybe_unused]] int const error = errno;
		std::abort();
	}
}

void test::virtual_decommit(void* const address, size_t const size)
{
	if (mprotect(address, size, PROT_NONE) == -1)
	{
		[[maybe_unused]] int const error = errno;
		std::abort();
	}
}
