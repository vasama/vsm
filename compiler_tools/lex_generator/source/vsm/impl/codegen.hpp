#pragma once

#include <vsm/impl/semantics.hpp>

namespace vsm::lexgen {

std::string generate(state_machine const& machine, std::string_view prefix);

} // namespace vsm::lexgen
