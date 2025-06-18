#pragma once

#include <cstddef>
#include <cstdint>

namespace vsm {

[[nodiscard]] inline size_t memalignment(void* const ptr)
{
	return reinterpret_cast<uintptr_t>(ptr) & ~reinterpret_cast<uintptr_t>(ptr) + 1;
}

void memswap(void* lhs, void* rhs, size_t size);

} // namespace vsm
