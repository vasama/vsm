#pragma once

#include <source_location>

namespace vsm::unit_test::backend {

uint32_t get_branch_position(std::source_location const& source);

} // namespace vsm::unit_test::backend
