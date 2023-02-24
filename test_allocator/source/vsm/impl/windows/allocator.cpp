#include <vsm/impl/allocator.hpp>

#include <cstdlib>

#include <Windows.h>

using namespace vsm;
using namespace vsm::test;

size_t test::virtual_page_size()
{
	SYSTEM_INFO system_info;
	GetSystemInfo(&system_info);
	return system_info.dwPageSize;
}

void* test::virtual_allocate(size_t const size)
{
	void* const address = VirtualAlloc(
		nullptr,
		size,
		MEM_RESERVE,
		PAGE_NOACCESS);

	if (address == nullptr)
	{
		[[maybe_unused]] DWORD const error = GetLastError();
		std::abort();
	}

	return address;
}

void test::virtual_deallocate(void* const address, size_t const size)
{
	if (!VirtualFree(
		address,
		0,
		MEM_RELEASE))
	{
		[[maybe_unused]] DWORD const error = GetLastError();
		std::abort();
	}
}

void test::virtual_commit(void* const address, size_t const size)
{
	if (VirtualAlloc(
		address,
		size,
		MEM_COMMIT,
		PAGE_READWRITE) == nullptr)
	{
		[[maybe_unused]] DWORD const error = GetLastError();
		std::abort();
	}
}

void test::virtual_decommit(void* const address, size_t const size)
{
	if (!VirtualFree(
		address,
		size,
		MEM_DECOMMIT))
	{
		[[maybe_unused]] DWORD const error = GetLastError();
		std::abort();
	}
}
