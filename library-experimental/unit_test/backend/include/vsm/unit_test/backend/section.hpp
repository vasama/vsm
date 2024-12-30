#pragma once

#include <source_location>
#include <string_view>

namespace vsm::unit_test::backend {

void enter_section_set(std::source_location const& source, std::string_view name);
void leave_section_set(std::source_location const& source);

bool enter_section(std::source_location const& source, std::string_view name);
void leave_section(std::source_location const& source);

} // namespace vsm::unit_test::backend
