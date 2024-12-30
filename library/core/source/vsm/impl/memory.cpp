#include <vsm/memory.hpp>

#include <cstring>

void vsm::memswap(void* const lhs, void* const rhs, size_t const size)
{
	static constexpr size_t block_size = 64;

	auto const l = static_cast<unsigned char*>(lhs);
	auto const r = static_cast<unsigned char*>(rhs);

	alignas(block_size) unsigned char buffer[block_size];
	size_t offset = 0;

	for (size_t const block_end = size - size % block_size; offset < block_end; offset += block_size)
	{
		memcpy(buffer, l + offset, block_size);
		memcpy(l + offset, r + offset, block_size);
		memcpy(r + offset, buffer, block_size);
	}

	if (size_t const remaining = size - offset)
	{
		memcpy(buffer, l + offset, remaining);
		memcpy(l + offset, r + offset, remaining);
		memcpy(r + offset, buffer, remaining);
	}
}
