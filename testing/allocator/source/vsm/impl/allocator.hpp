#pragma once

#include <cstddef>

namespace vsm::test {

size_t virtual_page_size();
void* virtual_allocate(size_t size);
void virtual_deallocate(void* address, size_t size);
void virtual_commit(void* address, size_t size);
void virtual_decommit(void* address, size_t size);

} // namespace vsm::test
